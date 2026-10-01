#include "Character.h"
#include "../../DataLoader/DataManager.h"
#include "../../DataLoader/AttackData.h"
Character::Character()
{
}

Character::~Character()
{
}

void Character::Init()
{

}

void Character::Update()
{

}

void Character::Draw()
{

}

void Character::Damage(int damage)
{
	m_hp -= damage;
}

void Character::ApplyDamage(int damage)
{
	//ジャスト回避受付中ならジャスト回避成功
	if (m_isJustDodgeWindow && damage <= 0)
	{
		OnJustDodge();
		return;
	}
	//無敵中はダメージを受けない
	if (m_isInvincible) return;

	Damage(damage);
}

void Character::OnHit(const AttackData& attackData)
{
	ApplyDamage(attackData.GetDamage());
}

Character::Capsule Character::GetCapsuleAt(const Vector3& footPos) const
{
	Capsule capsule;
	capsule.radius = m_collisionRadius;
	//下の球の中心は足元から半径分上
	capsule.start = footPos + Vector3(0.0f, m_collisionRadius, 0.0f);
	//上の球の中心は頭のてっぺんから半径分下
	//高さが半径の2倍より小さい場合は球として扱う
	float endY = m_collisionHeight - m_collisionRadius;
	if (endY < m_collisionRadius) endY = m_collisionRadius;
	capsule.end = footPos + Vector3(0.0f, endY, 0.0f);
	return capsule;
}

bool Character::IsDead() const
{
	return m_isDead;
}

void Character::StopMove()
{
	m_velocity = Vector3(0.0f, m_velocity.y, 0.0f);
}

void Character::ChangeState(std::unique_ptr<StateBase> nextState)
{
	m_pNextState = std::move(nextState);
}

StateBase::StateKind Character::GetStateKind() const
{
	return m_pState->GetKind();
}

void Character::UpdateState()
{
	//外部(死亡判定など)から予約されたステートを先に反映する
	ApplyNextState();

	if (m_pState)
	{
		m_pState->Update();
	}

	//Update中に予約されたステートを反映する
	ApplyNextState();
}

void Character::ApplyNextState()
{
	if (!m_pNextState) return;

	if (m_pState)
	{
		m_pState->Exit();
	}
	m_pState = std::move(m_pNextState);
	m_pState->Enter();
}

void Character::LoadStatus(const std::string& name)
{
	auto& status = DataManager::GetInstance().GetStatusData().GetStatus(name);
	m_maxhp = status.maxhp;
	m_hp = status.maxhp;//初期HPは最大値から使用
	m_speed = status.speed;
	m_attackPower = status.attackPower;
}
