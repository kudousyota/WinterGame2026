#include "Player.h"
#include "../../../Common/Model.h"
#include "../../../System/Timer.h"
#include "State/PlayerStateIdle.h"
#include "State/PlayerStateDeath.h"
#include "State/PlayerStateDamage.h"
#include <cmath>
#include "../../../System/Input.h"
#include "../../../Math/MathUtil.h"
#include "../../../DataLoader/DataManager.h"

namespace
{
	const Vector3 kPlayerToTarget = Vector3(0.0f, 200.0f, 0.0f);

	//重力や回転などの手触りに関わる数値はPlayerParam.csvに書く

	//アニメション
	const char* const kIdleAnimName = "Player|Idle";

	//体の大きさ
	//身長
	constexpr float kHeight = 180.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 30.0f;

	//被弾後の無敵中に点滅する間隔
	constexpr int kBlinkInterval = 4;
}

Player::Player()
{
}

Player::~Player()
{
	//モデルの解放
	MV1DeleteModel(m_modelHandle);
}

void Player::Init()
{
	//モデルの生成
	m_modelHandle = Model::Instance().CreatPlayerModel();
	//アニメションの初期化
	m_anim.Init(m_modelHandle, kIdleAnimName, true);

	//ステータスの初期化
	LoadStatus("Player");

	m_pos = Vector3(0.0f, 0.0f, 0.0f);
	//最初は+Zを向く
	m_forward = Vector3(0.0f, 0.0f, 1.0f);
	m_angle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	
	//体の大きさ
	m_height = kHeight;
	m_collisionRadius = kCollisionRadius;
	//重力とジャスト回避の半径はCSVから入れる
	ApplyParam();
	MV1SetPosition(m_modelHandle, m_pos);
	//向きもすぐ反映する
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));

	//最初はIdleで初期化
	ChangeState(std::make_unique<PlayerStateIdle>(*this));
}

void Player::Update()
{
	//HPが0以下になったら強制的に死亡ステートにする
	if (m_hp <= 0 && !m_isDead)
	{
		m_isDead = true;
		ChangeState(std::make_unique<PlayerStateDeath>(*this));
	}

	//被弾後の無敵の残りを減らす
	if (m_hitInvincibleFrame > 0)
	{
		m_hitInvincibleFrame--;
	}

	//地面にいる間は浮ける権利を戻す
	if (IsGround())
	{
		m_canAirHover = true;
		m_canDoubleJump = true;
	}

	//ステートの更新
	UpdateState();
	//アニメーションの更新
	if(!m_isAnimPause)
	{
		m_anim.Update(Timer::Instance().GetTimeScale());
	}
	//重力
	m_velocity.y -= m_gravity * m_gravityScale;

	//移動
	//地面との判定は全員が動き終わった後にGameSceneがステージとまとめて行う
	m_pos += m_velocity;

	//移動している方向に体を向ける
	Vector3 moveXZ(m_velocity.x, 0.0f, m_velocity.z);
	if (moveXZ.SqMagnitude() > 0.0f)
	{
		m_forward = moveXZ.Normalize();
	}
	//モデルは-Z方向に向きを合わせる
	float targetAngle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;

	//遠回りして回転しないようにする
	float diff = MathUtil::WrapAngle(targetAngle - m_angle);

	//差の一部だけ回転して、少しずつ目標の向きに近づける
	m_angle += diff * DataManager::GetInstance().GetPlayerParam().rotateRate;
}

void Player::ReloadStatus()
{
	//HPと速度はキャラクター共通の読み直しで反映する
	Character::ReloadStatus();
	//メンバーに持っているCSVの値も入れ直す
	ApplyParam();
}

void Player::ApplyParam()
{
	const auto& param = DataManager::GetInstance().GetPlayerParam();
	m_gravity = param.gravity;
	m_justDodgeRadius = param.justDodgeRadius;
}

void Player::LateUpdate()
{
	//当たり判定で押し出された後の位置をモデルに反映する
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));
	MV1SetPosition(m_modelHandle, m_pos);
}

void Player::Draw()
{
	MV1DrawModel(m_modelHandle);
}

HitResult Player::OnHit(const AttackData& attackData, int damage)
{
	//無敵中と死亡中は当たっていないのでのけぞらない
	if (!CanBeHit()) return HitResult::Ignored;

	//このダメージで倒れた時はのけぞらない
	HitResult result = Character::OnHit(attackData, damage);
	if (result == HitResult::Killed) return result;

	//しばらく無敵にして、多段攻撃で動けなくならないようにする
	m_hitInvincibleFrame = DataManager::GetInstance().GetPlayerParam().hitInvincibleFrame;
	ChangeState(std::make_unique<PlayerStateDamage>(*this));
	return HitResult::Flinch;
}

void Player::OnJustDodge()
{
	//敵だけ時間を遅くする
	const auto& param = DataManager::GetInstance().GetPlayerParam();
	Timer::Instance().SetEnemyTimeScaleForFrames(param.witchTimeScale, param.witchTimeFrame);
}

Vector3 Player::GetCameraTarget()
{
	return m_pos + kPlayerToTarget;
}

void Player::SetCameraAxis(const Vector3& forward, const Vector3& right)
{
	m_cameraForward = forward;
	m_cameraRight = right;
}

void Player::FaceTo(const Vector3& dir)
{
	//判定の向きも変える
	m_forward = dir;
	//モデルの向きもすぐ合わせる
	m_angle = atan2f(dir.x, dir.z) + DX_PI_F;
}

Vector3 Player::GetMoveInput()const
{
	auto& input = Input::Instance();

	//左スティックの入力
	Vector3 dir(input.GetStickLX(), 0.0f, input.GetStickLY());
	//スティックが倒れていない時だけ十字キーを見る
	if (dir.SqMagnitude() <= 0.0f)
	{
		//十字キーの入力
		if (input.IsPressed("right")) dir.x += 1.0f;
		if (input.IsPressed("left"))  dir.x -= 1.0f;
		if (input.IsPressed("up"))    dir.z += 1.0f;
		if (input.IsPressed("down"))  dir.z -= 1.0f;
	}
	//斜め入力で速くならないよう長さを1以下に抑える
	if (dir.SqMagnitude() > 1.0f)
	{
		dir = dir.Normalize();
	}

	//入力をカメラ基準の方向に変換する
	return m_cameraRight * dir.x + m_cameraForward * dir.z;
}

void Player::UseAirHover()
{
	m_canAirHover = false;
}

void Player::UseDoubuleJump()
{
	m_canDoubleJump = false;
}
