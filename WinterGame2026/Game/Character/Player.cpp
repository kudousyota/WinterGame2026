#include "Player.h"
#include "../Comon/Model.h"
#include "State/PlayerStateIdle.h"
#include "State/PlayerStateDeath.h"
#include <cmath>

namespace
{
	const Vector3 kPlayerToTarget = Vector3(0.0f, 200.0f, 0.0f);

	//重力
	constexpr float kGravity = 0.5f;
	//地面の高さ
	constexpr float kGroundY = 0.0f;

	//回転の補完
	constexpr float kRotateRate = 0.2f;

	//アニメション
	const char* const kIdleAnimName = "Player|Idle";

	//体の大きさ(デバッグ表示のカプセルを見ながら調整する)
	//身長
	constexpr float kHeight = 180.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 30.0f;
	//ジャスト回避判定の半径
	constexpr float kJustDodgeRadius = 50.0f;
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
	//最初はカメラの方を向かせる
	m_forward = Vector3(0.0f, 0.0f, -1.0f);
	m_gravity = kGravity;
	//体の大きさ
	m_height = kHeight;
	m_collisionRadius = kCollisionRadius;
	m_justDodgeRadius = kJustDodgeRadius;
	MV1SetPosition(m_modelHandle, m_pos);

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

	//ステートの更新
	UpdateState();
	//アニメーションの更新
	m_anim.Update(1.0f);


	//重力
	m_velocity.y -= m_gravity;
	//移動
	m_pos += m_velocity;

	//地面との判定
	if (m_pos.y <= kGroundY)
	{
		m_pos.y = kGroundY;
		m_velocity.y = 0.0f;
		m_isGround = true;
	}
	else
	{
		m_isGround = false;
	}

	//移動している方向に体を向ける
	Vector3 moveXZ(m_velocity.x, 0.0f, m_velocity.z);
	if (moveXZ.SqMagnitude() > 0.0f)
	{
		m_forward = moveXZ.Normalize();
	}
	//モデルは-Z方向に向きを合わせる
	float targetAngle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;

	//目標の角度との差を-π~πにおさめる(遠回りして回転しないようにする)
	float diff = targetAngle - m_angle;
	while (diff > DX_PI_F)  diff -= DX_TWO_PI_F;
	while (diff < -DX_PI_F) diff += DX_TWO_PI_F;

	//差の一部だけ回転して、少しずつ目標の向きに近づける
	m_angle += diff * kRotateRate;
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

Vector3 Player::GetCameraTarget()
{
	return m_pos + kPlayerToTarget;
}

void Player::SetCameraAxis(const Vector3& forward, const Vector3& right)
{
	m_cameraForward = forward;
	m_cameraRight = right;
}