#include "Camera.h"
#include <cmath>
#include "DxLib.h"
#include "../System/StageCollision.h"
#include "../System/Input.h"

namespace
{
	//視野角
	constexpr float kFov = DX_PI_F / 3.0f;
	//映る一番近い距離と遠い距離(ステージの端から端まで映るようにする)
	//大聖堂の時計塔(高さ約80m)の先まで映るように200mにしている
	constexpr float kNear = 200.0f;
	constexpr float kFar = 20000.0f;

	//注視点からカメラまでの距離
	constexpr float kDistance = 500.0f;
	//注視点が追いかける速さ(0～1) 
	constexpr float kFocusFollow = 0.15f;

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
	//ロックオン中の敵の方へ回る速さ
	constexpr float kLockOnRotSpeed = 0.1f;
	//敵がこれより近いと向きが安定しないので回さない
	constexpr float kLockOnMindistance = 50.0f;

	//注視点をプレイヤーから敵の方へずらす割合
	constexpr float kLockOnFocusRate = 0.3f;
	//ずらす量の上限
	constexpr float kLockOnFocusMax = 150.0f;
}

void Camera::Init(const Vector3& focusPos, float yaw)
{
	SetupCamera_Perspective(kFov);
	SetCameraNearFar(kNear, kFar);

	m_yaw = yaw;
	m_pitch = 0.0f;
	m_pullOffset = 0.0f;
	m_focus = focusPos;
	m_pos = m_focus - CalcDir() * kDistance;

	Apply();
}

void Camera::Update(const Vector3& focusPos, const std::optional<Vector3>& lockOnPos)
{
	//角度を決める
	if (lockOnPos)
	{
		UpdateLockOnAngle(focusPos, *lockOnPos);
	}
	else
	{
		UpdateFreeAngle();
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
		focusGoal += shift;
	}
	m_focus += (focusGoal - m_focus) * kFocusFollow;

	//角度と距離から位置を出し、ステージにめり込まないようにする
	Vector3 idealPos = m_focus - CalcDir() * (kDistance + m_pullOffset);
	m_pos = AvoidStage(idealPos);

	Apply();
}

void Camera::UpdateFreeAngle()
{
	auto& input = Input::Instance();
	m_yaw += input.GetStickRX() * kRotSpeedYaw;
	//スティックを上に倒すとカメラが下から見上げる(作り直す前と同じ向き)
	m_pitch += input.GetStickRY() * kRotSpeedPitch;
}

void Camera::UpdateLockOnAngle(const Vector3& focusPos, const Vector3& lockOnPos)
{
	//プレイヤーから敵への水平の大きさ
	Vector3 toEnmey = lockOnPos - focusPos;
	toEnmey.y = 0.0f;

	//近づくと,少しだけカメラが震えるので回さない
	if (toEnmey.SqMagnitude() < kLockOnMindistance * kLockOnMindistance) return;

	float targetYaw = atan2f(toEnmey.x, toEnmey.z);

	//遠回りしないように
	float diff = targetYaw - m_yaw;
	if (diff > DX_PI_F)  diff -= DX_TWO_PI_F;
	if (diff < -DX_PI_F) diff += DX_TWO_PI_F;

	//差の一部だけ回す
	m_yaw += diff * kLockOnRotSpeed;

}

void Camera::ClampAngle()
{
	if (m_pitch > kMaxPitch) m_pitch = kMaxPitch;
	if (m_pitch < kMinPitch) m_pitch = kMinPitch;

	if (m_yaw > DX_PI_F)  m_yaw -= DX_TWO_PI_F;
	if (m_yaw < -DX_PI_F) m_yaw += DX_TWO_PI_F;
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

Vector3 Camera::AvoidStage(const Vector3& idealPos) const
{
	if (m_stageHandle < 0) return idealPos;

	//注視点からカメラへレイを飛ばし、間に壁があればその手前に置く
	Vector3 hitPos;
	if (!StageCollision::CheckCameraRay(m_stageHandle, m_focus, idealPos, hitPos))
	{
		return idealPos;
	}
	Vector3 toFocus = (m_focus - hitPos).Normalize();
	return hitPos + toFocus * kStageMargin;
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
