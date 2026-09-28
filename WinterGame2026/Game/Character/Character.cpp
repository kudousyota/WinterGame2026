#include "Character.h"
#include "../../DataLoader/DataManager.h"
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

bool Character::IsDead() const
{
	return m_isDead;
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
