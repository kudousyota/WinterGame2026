#include "Character.h"
#include "DxLib.h"
#include <cmath>
#include "../../DataManager.h"

namespace
{
	//重力加速度
	constexpr float kGravity = -0.5f;
	//落下速度の上限
	constexpr float kMaxFallSpeed = -20.0f;
	//地面の高さ(ステージができたら当たり判定に置き換える)
	constexpr float kGroundHeight = 0.0f;
	//ダメージを受けた後の無敵フレーム
	constexpr int kInvincibleFrame = 60;
}

Character::Character(const std::string& name) :
	m_name(name),
	m_modelHandle(-1),
	m_pos(),
	m_forward(0.0f, 0.0f, 1.0f),
	m_velocity(),
	m_speed(0.0f),
	m_gravity(kGravity),
	m_hp(0),
	m_maxhp(0),
	m_isGround(false),
	m_isDamage(false),
	m_isInvincible(false),
	m_invincibleFrame(0),
	m_isDead(false)
{
}

Character::~Character()
{
	if (m_modelHandle != -1)
	{
		MV1DeleteModel(m_modelHandle);
	}
}

void Character::Init()
{
	//CSVからステータスを取得
	const auto& status = DataManager::GetInstance().GetStatusData().GetStatus(m_name);
	m_maxhp = status.maxhp;
	m_hp = m_maxhp;
	m_speed = status.speed;

	//状態をリセット
	m_velocity = Vector3();
	m_isGround = false;
	m_isDamage = false;
	m_isInvincible = false;
	m_invincibleFrame = 0;
	m_isDead = false;
}

void Character::Update()
{
	UpdateMove();
	UpdateInvincible();
}

void Character::Draw()
{
	if (m_modelHandle == -1) return;

	MV1SetPosition(m_modelHandle, m_pos.ToDxLibVector());
	//前方向にモデルを向ける
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, atan2f(-m_forward.x, -m_forward.z), 0.0f));
	MV1DrawModel(m_modelHandle);
}

void Character::Damage(int damage)
{
	//死亡中・無敵中は食らわない
	if (m_isDead || m_isInvincible) return;

	m_hp -= damage;
	m_isDamage = true;

	if (m_hp <= 0)
	{
		m_hp = 0;
		m_isDead = true;
		OnDead();
		return;
	}

	//一定時間無敵にする
	m_isInvincible = true;
	m_invincibleFrame = kInvincibleFrame;
}

bool Character::IsDead() const
{
	return m_isDead;
}

void Character::LoadModel(const char* path)
{
	if (m_modelHandle != -1)
	{
		MV1DeleteModel(m_modelHandle);
	}
	m_modelHandle = MV1LoadModel(path);
}

void Character::UpdateMove()
{
	//重力
	m_velocity.y += m_gravity;
	if (m_velocity.y < kMaxFallSpeed)
	{
		m_velocity.y = kMaxFallSpeed;
	}

	m_pos += m_velocity;

	//接地判定
	if (m_pos.y <= kGroundHeight)
	{
		m_pos.y = kGroundHeight;
		m_velocity.y = 0.0f;
		m_isGround = true;
	}
	else
	{
		m_isGround = false;
	}
}

void Character::UpdateInvincible()
{
	if (!m_isInvincible) return;

	m_invincibleFrame--;
	if (m_invincibleFrame <= 0)
	{
		m_invincibleFrame = 0;
		m_isInvincible = false;
		m_isDamage = false;
	}
}
