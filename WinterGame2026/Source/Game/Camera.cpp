#include "Camera.h"
#include <cmath>
#include <algorithm>
#include "DxLib.h"
#include "../System/StageCollision.h"
#include "../System/Input.h"
#include "../Math/MathUtil.h"

namespace
{
	//視野角
	constexpr float kFov = DX_PI_F / 3.0f;
	//映る一番近い距離と遠い距離ステージの端から端まで映るようにする
	constexpr float kNear = 20.0f;
	constexpr float kFar = 20000.0f;

	//注視点からカメラまでの距離
	constexpr float kDistance = 500.0f;
	//注視点が追いかける速さ(0～1) 
	constexpr float kFocusFollow = 0.15f;
	//ジャンプでカメラが揺れない用に遅くする
	constexpr float kFocusFollowY = 0.04f;
	//注視点が縦に遅れてよい最大の量
	constexpr float kFocusMaxLagY = 120.0f;
	//右スティックで回る速さ
	constexpr float kRotSpeedYaw = 0.037f;
	constexpr float kRotSpeedPitch = 0.03f;
	//縦回転の制限
	constexpr float kMaxPitch = DX_PI_F * 0.4f;
	constexpr float kMinPitch = -DX_PI_F * 0.2f;

	//引いたときに足す距離
	constexpr float kPullDistance = 150.0f;
	//引く・戻る速さ(0～1)
	constexpr float kPullSpeed = 0.3f;

	//ステージに当たったとき、壁から離す距離
	constexpr float kStageMargin = 10.0f;
	//壁から離れた時に戻る速さ
	constexpr float kReturnSpeed = 0.1f;
	//ロックオン中の敵の方へ回る速さ
	constexpr float kLockOnRotSpeed = 0.1f;
	//カメラリセットで背中側へ戻る速さ(0～1)
	//一瞬で切り替えると向きを見失うので、回っていく様子が見える程度に速くする
	constexpr float kRecenterSpeed = 0.2f;
	//目標との差がこれより小さくなったら戻し終わったとみなす(ラジアン)
	constexpr float kRecenterEndAngle = 0.01f;
	//カメラリセットで戻す縦の角度
	constexpr float kRecenterPitch = 0.0f;
	//敵がこれより近いと向きが安定しないので回さない
	constexpr float kLockOnMindistance = 50.0f;

	//ロックオン中、プレイヤーと敵の中点を見る(0.5で中点)
	constexpr float kLockOnFocusRate = 0.5f;
	//ずらす量の上限(敵が遠いとき、プレイヤーが画面の端へ行きすぎないように)
	constexpr float kLockOnFocusMax = 450.0f;

	//ロックオン中、敵の方向からカメラをずらす角度
	//大きくしすぎると、スティックの上で敵へまっすぐ向かわなくなる
	constexpr float kLockOnSideAngle = DX_PI_F / 3.0f;
	//ロックオン中の縦の角度(マイナスで少し上から見下ろす。0で水平)
	//ロックオン前にどんな角度でも、この角度にそろえて同じ構図にする
	constexpr float kLockOnPitch = -0.05f;

	//ロックオン中、注視点から奥に1進むと、画面の横のどこまで入れてよいか
	//小さいほど2人が画面の中央寄りに収まり、カメラは引く(画面の端ぎりぎりは約1.0)
	constexpr float kScreenSideRate = 0.45f;
	//同じく縦(画面は横長なので横より狭い。画面の端ぎりぎりは約0.58)
	constexpr float kScreenUpRate = 0.3f;
	//2人を映す距離へ寄る・引く速さ(0～1)
	constexpr float kFitSpeed = 0.1f;

	//これ以下の身長は普通の敵(見上げない)
	constexpr float kNormalEnemyHeight = 250.0f;
	//これ以上の身長は一番見上げる
	constexpr float kBigEnemyHeight = 1000.0f;
	//一番見上げるときの縦の角度(プラスで下から見上げる)
	//大きくしすぎるとカメラが床に当たって急に寄る
	constexpr float kLockOnBigPitch = 0.15f;
	//大きい敵のとき、注視点を敵の真ん中の高さへどれだけ近づけるか(0～1)
	//上げないと、敵を画面に入れるためにカメラが引いて床まで下がってしまう
	constexpr float kBigEnemyFocusUpRate = 0.5f;

	//敵の大きさを0～1にする(普通の敵は0、一番大きい敵は1)
	float CalcBigRate(float height)
	{
		float rate = (height - kNormalEnemyHeight) / (kBigEnemyHeight - kNormalEnemyHeight);
		return std::clamp(rate, 0.0f, 1.0f);
	}
}

void Camera::Init(const Vector3& focusPos, const Vector3& forward)
{
	SetupCamera_Perspective(kFov);
	SetCameraNearFar(kNear, kFar);

	//プレイヤーの正面の向きから角度を出す(プレイヤーのm_angleはモデル用に180度ずれているので使わない)
	m_yaw = atan2f(forward.x, forward.z);
	m_pitch = 0.0f;
	m_pullOffset = 0.0f;
	m_focus = focusPos;
	m_baseDistance = kDistance;
	m_curDistance = kDistance;
	m_pos = m_focus - CalcDir() * m_curDistance;

	Apply();
}

void Camera::Update(const Vector3& focusPos, const Vector3& PlayerForward, const std::optional<Vector3>& lockOnPos, float lockOnHeight)
{
	//角度を決める
	if (lockOnPos)
	{
		UpdateLockOnAngle(focusPos, *lockOnPos, lockOnHeight);
		m_noInputFrame = 0;
		//ロックオン中は敵の方を向くのが優先なので、カメラリセットはやめる
		m_isRecentering = false;
	}
	else
	{
		UpdateFreeAngle();
		UpdateRecenter(PlayerForward);
	}
	ClampAngle();

	//距離を決める
	UpdateDistance();

	//注視点を追いかける
	Vector3 focusGoal = focusPos;

	if (lockOnPos)
	{
		Vector3 shift = (*lockOnPos - focusPos) * kLockOnFocusRate;
		shift.y = 0.0f;
		if (shift.SqMagnitude() > kLockOnFocusMax * kLockOnFocusMax)
		{
			shift = shift.Normalize() * kLockOnFocusMax;
		}
		//大きい敵のときは注視点を敵の真ん中の高さへ近づける(普通の敵は上げない)
		shift.y = (lockOnPos->y - focusPos.y) * kBigEnemyFocusUpRate * CalcBigRate(lockOnHeight);
		focusGoal += shift;
	}
	m_focus.x += (focusGoal.x - m_focus.x) * kFocusFollow;
	m_focus.y += (focusGoal.y - m_focus.y) * kFocusFollowY;
	m_focus.z += (focusGoal.z - m_focus.z) * kFocusFollow;

	//離れすぎたら上限の位置まで戻す
	float lagY = m_focus.y - focusGoal.y;
	//上に離れすぎたら
	if (lagY > kFocusMaxLagY)
	{
		m_focus.y = focusGoal.y + kFocusMaxLagY;
	}
	//下に離れすぎたら
	else if (lagY < -kFocusMaxLagY)
	{
		m_focus.y = focusGoal.y - kFocusMaxLagY;
	}

	Vector3 dir = CalcDir();

	//ロックオン中は、プレイヤーと敵の両方が画面の内側に入る距離まで下がる
	float targetDist = kDistance;
	if (lockOnPos)
	{
		targetDist = (std::max)(targetDist, CalcFitDistance(focusPos, dir));
		targetDist = (std::max)(targetDist, CalcFitDistance(*lockOnPos, dir));
	}
	//急に寄ったり引いたりしないように補間する
	m_baseDistance += (targetDist - m_baseDistance) * kFitSpeed;

	//角度と距離から位置を出し、ステージにめり込まないようにする
	float idealDist = m_baseDistance + m_pullOffset;
	float limit = CalcStageDistance(dir, idealDist);
	if (limit < m_curDistance)
	{
		m_curDistance = limit;
	}
	else
	{
		m_curDistance += (limit - m_curDistance) * kReturnSpeed;
	}
	m_pos = m_focus - dir * m_curDistance;

	Apply();
}

void Camera::UpdateFreeAngle()
{
	auto& input = Input::Instance();
	m_yaw += input.GetStickRX() * kRotSpeedYaw;
	//スティックを上に倒すとカメラが下から見上げる
	m_pitch += input.GetStickRY() * kRotSpeedPitch;
	if (input.IsRightStickActive())
	{
		m_noInputFrame = 0;
		//自分で動かし始めたら、カメラリセットより操作を優先する
		m_isRecentering = false;
	}
	else
	{
		m_noInputFrame++;
	}

	//右スティック押し込みで背中側へ戻し始める
	if (input.IsTriggered("cameraReset"))
	{
		m_isRecentering = true;
	}
}

void Camera::UpdateRecenter(const Vector3& playerForward)
{
	//戻している途中でなければ何もしない
	if (!m_isRecentering) return;

	//プレイヤーの正面の角度
	float targetYaw = atan2f(playerForward.x, playerForward.z);

	//遠回りしないように
	float diffYaw = MathUtil::WrapAngle(targetYaw - m_yaw);
	float diffPitch = kRecenterPitch - m_pitch;

	//差の一部だけ回す
	m_yaw += diffYaw * kRecenterSpeed;
	m_pitch += diffPitch * kRecenterSpeed;

	//横も縦もほぼ目標に着いたら、戻し終わり
	if (fabsf(diffYaw) < kRecenterEndAngle && fabsf(diffPitch) < kRecenterEndAngle)
	{
		m_isRecentering = false;
	}
}

void Camera::UpdateLockOnAngle(const Vector3& focusPos, const Vector3& lockOnPos, float lockOnHeight)
{
	//大きい敵ほど下から見上げる角度にする
	float targetPitch = kLockOnPitch + (kLockOnBigPitch - kLockOnPitch) * CalcBigRate(lockOnHeight);

	//縦の角度は決まった値へ少しずつ寄せる(敵が近くて横を回さないときも寄せる)
	m_pitch += (targetPitch - m_pitch) * kLockOnRotSpeed;

	//プレイヤーから敵への水平の大きさ
	Vector3 toEnmey = lockOnPos - focusPos;
	toEnmey.y = 0.0f;

	//近づくと少しだけカメラが震えるので回さない
	if (toEnmey.SqMagnitude() < kLockOnMindistance * kLockOnMindistance) return;

	//敵がいる方向の角度
	float enemyYaw = atan2f(toEnmey.x, toEnmey.z);

	//敵の方向から右と左にずらした角度までの差
	float diffR = MathUtil::WrapAngle(enemyYaw + kLockOnSideAngle - m_yaw);
	float diffL = MathUtil::WrapAngle(enemyYaw - kLockOnSideAngle - m_yaw);

	//今のカメラに近い方を目標にする
	float diff = (fabsf(diffR) < fabsf(diffL)) ? diffR : diffL;

	//差の一部だけ回す
	m_yaw += diff * kLockOnRotSpeed;
}

void Camera::ClampAngle()
{
	if (m_pitch > kMaxPitch) m_pitch = kMaxPitch;
	if (m_pitch < kMinPitch) m_pitch = kMinPitch;

	m_yaw = MathUtil::WrapAngle(m_yaw);
}

void Camera::UpdateDistance()
{
	float goal = m_isPull ? kPullDistance : 0.0f;
	m_pullOffset += (goal - m_pullOffset) * kPullSpeed;

	//ほぼ戻ったら0にする
	if (!m_isPull && fabsf(m_pullOffset) < 0.01f)
	{
		m_pullOffset = 0.0f;
	}
}

Vector3 Camera::CalcDir() const
{
	return Vector3(
		sinf(m_yaw) * cosf(m_pitch),
		sinf(m_pitch),
		cosf(m_yaw) * cosf(m_pitch));
}

float Camera::CalcFitDistance(const Vector3& pos, const Vector3& dir) const
{
	//カメラの水平な正面と右
	//縦の角度はロックオン中ほぼ水平なので、高さは別に見る
	Vector3 forward(dir.x, 0.0f, dir.z);
	if (forward.SqMagnitude() <= 0.0f) return 0.0f;
	forward = forward.Normalize();
	Vector3 right(forward.z, 0.0f, -forward.x);

	//注視点からposへのずれを、奥・横・高さに分ける
	Vector3 toPos = pos - m_focus;
	float depth = toPos.Dot(forward);
	float side = fabsf(toPos.Dot(right));
	float up = fabsf(toPos.y);

	//横も高さも画面の内側に収まるまで下がる
	//奥にあるもの(depthがプラス)はその分カメラから遠いので、少し近くても収まる
	float need = (std::max)(side / kScreenSideRate, up / kScreenUpRate);
	return need - depth;
}

float Camera::CalcStageDistance(const Vector3& dir, float idealDist) const
{
	//ステージがなければ位置を変えない
	if (m_stageHandle < 0) return idealDist;
	//本来のカメラの位置
	Vector3 idealPos = m_focus - dir * idealDist;

	//注視点からカメラへレイを飛ばし、間に壁があればその手前に置く
	Vector3 hitPos;
	if (!StageCollision::CheckCameraRay(m_stageHandle, m_focus, idealPos, hitPos))
	{
		return idealDist;
	}
	return (hitPos - m_focus).Magnitude() - kStageMargin;
}

void Camera::Apply()
{
	//揺れは見た目だけなので、m_posには足さない
	Vector3 shakeOffset = CalcShakeOffset();
	if (m_shakeFrame > 0)
	{
		m_shakeFrame--;
	}
	Vector3 pos = m_pos + shakeOffset;
	Vector3 target = m_focus + shakeOffset;

	SetCameraPositionAndTarget_UpVecY(pos.ToDxLibVector(), target.ToDxLibVector());

	//ライトはカメラの向きから当てる
	SetLightDirection((target - pos).Normalize().ToDxLibVector());
}

Vector3 Camera::GetForward() const
{
	return Vector3(sinf(m_yaw), 0.0f, cosf(m_yaw));
}

Vector3 Camera::GetRight() const
{
	//正面を右に90度回した向き
	return Vector3(cosf(m_yaw), 0.0f, -sinf(m_yaw));
}

void Camera::Shake(float power, int frame)
{
	//今の揺れより弱い揺れでは上書きしない
	if (m_shakeFrame > 0 && power < m_shakePower) return;

	m_shakePower = power;
	m_shakeFrame = frame;
	m_shakeTotalFrame = frame;
}

Vector3 Camera::CalcShakeOffset() const
{
	if (m_shakeFrame <= 0 || m_shakeTotalFrame <= 0) return Vector3(0.0f, 0.0f, 0.0f);

	//残り時間に合わせてだんだん弱くする
	float power = m_shakePower * static_cast<float>(m_shakeFrame) / static_cast<float>(m_shakeTotalFrame);

	//-power～powerのランダムな値
	auto randRange = [power]()
		{
			return (static_cast<float>(GetRand(2000)) / 1000.0f - 1.0f) * power;
		};
	return Vector3(randRange(), randRange(), randRange());
}
