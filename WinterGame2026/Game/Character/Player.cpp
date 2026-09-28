#include "Player.h"
#include "../Comon/Model.h"
#include "State/PlayerStateIdle.h"
#include "State/PlayerStateDeath.h"
#include <cmath>

namespace
{
	const Vector3 kPlayerToTarget = Vector3(0.0f, 200.0f, 800.0f);

	//重力
	constexpr float kGravity = 0.5f;
	//地面の高さ
	constexpr float kGroundY = 0.0f;
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

	//ステータスの初期化
	LoadStatus("Player");

	m_pos = Vector3(0.0f, 0.0f, 0.0f);
	//最初はカメラの方を向かせる
	m_forward = Vector3(0.0f, 0.0f, -1.0f);
	m_gravity = kGravity;
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
	float angle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, angle, 0.0f));
	MV1SetPosition(m_modelHandle, m_pos);
}

void Player::Draw()
{
	MV1DrawModel(m_modelHandle);
#ifdef _DEBUG
	//現在のステートを表示
	if (m_pState)
	{
		DrawFormatString(0, 32, GetColor(255, 255, 255), "Player:%s", m_pState->GetKindName());
	}
#endif // _DEBUG
}

Vector3 Player::GetCameraTarget()
{
	return m_pos + kPlayerToTarget;
}
