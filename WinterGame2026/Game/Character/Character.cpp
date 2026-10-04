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

void Character::OnHit(const AttackData& attackData, int damage)
{
	//無敵中・死亡中はダメージを受けない
	if (!CanBeHit()) return;
	Damage(damage);
}

std::vector<AttackRequest> Character::TakeAttackRequests()
{
	std::vector<AttackRequest> requests;
	requests.swap(m_attackRequests);
	return requests;
}

Vector3 Character::GetAttackPos(const AttackRequest& request) const
{
	//足元から前方と上方向にずらした位置
	return m_pos + m_forward * request.forwardOffset + Vector3(0.0f, request.height, 0.0f);
}

bool Character::HasReceivedAttack(const Character* attacker, int attackId) const
{
	auto it = m_lastReceivedAttackId.find(attacker);
	return it != m_lastReceivedAttackId.end() && it->second == attackId;
}

void Character::MarkAttackReceived(const Character* attacker, int attackId)
{
	m_lastReceivedAttackId[attacker] = attackId;
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
	m_statusName = name;
	auto& status = DataManager::GetInstance().GetStatusData().GetStatus(name);
	m_maxhp = status.maxhp;
	m_hp = status.maxhp;//初期HPは最大値から使用
	m_speed = status.speed;
}

void Character::ReloadStatus()
{
	//ステータスを読み込んでいないキャラクターは何もしない
	if (m_statusName.empty()) return;

	auto& status = DataManager::GetInstance().GetStatusData().GetStatus(m_statusName);
	m_maxhp = status.maxhp;
	m_speed = status.speed;
	//最大HPが下がったときだけ今のHPを合わせる
	if (m_hp > m_maxhp) m_hp = m_maxhp;
}

#ifdef _DEBUG
std::string Character::GetDebugStateText() const
{
	if (!m_pState) return "State:none";

	std::string text = std::string("State:") + m_pState->GetKindName();
	std::string info = m_pState->GetDebugInfo();
	if (!info.empty())
	{
		text += "  " + info;
	}
	return text;
}
#endif
