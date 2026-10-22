#include "Character.h"
#include "../../DataLoader/DataManager.h"
#include "../../DataLoader/AttackData.h"
#include <algorithm>
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

std::vector<ShotRequest> Character::TakeShotRequests()
{
	std::vector<ShotRequest> requests;
	requests.swap(m_shotRequests);
	return requests;
}

Vector3 Character::GetAttackPos(const AttackRequest& request) const
{
	//足元から前方と上方向にずらした位置
	return m_pos + m_forward * request.forwardOffset + Vector3(0.0f, request.height, 0.0f);
}

Vector3 Character::GetShotPos(const ShotRequest& request) const
{
	return m_pos + m_forward * request.forwardOffset + Vector3(0.0f, request.height, 0.0f);
}

Geometry::Capsule Character::GetCollisionCapsule() const
{
	Geometry::Capsule capsule;
	//軸の下端は足元から半径分上げた位置
	capsule.start = m_pos + Vector3(0.0f, m_collisionRadius, 0.0f);
	capsule.end = capsule.start + Vector3(0.0f, GetCollisionHeight(), 0.0f);
	capsule.radius = m_collisionRadius;
	return capsule;
}

Geometry::Capsule Character::GetJustDodgeCapsule() const
{
	Geometry::Capsule capsule = GetCollisionCapsule();
	capsule.radius = m_justDodgeRadius;
	return capsule;
}

float Character::GetCollisionHeight() const
{
	return (std::max)(m_height - m_collisionRadius * 2.0f, 0.0f);
}

bool Character::HasReceivedAttack(const Character* attacker, int attackId) const
{
	auto it = m_receivedAttackIds.find(attacker);
	if (it == m_receivedAttackIds.end()) return false;
	//番号ごとに覚えておくので、足技・ウィケッドウィーブ・多段ヒットの各段が
	//同時に出ていても、それぞれ1回ずつ当たる
	const auto& ids = it->second;
	return std::find(ids.begin(), ids.end(), attackId) != ids.end();
}

void Character::MarkAttackReceived(const Character* attacker, int attackId)
{
	//同時に出ている攻撃は数個なので、古い番号は捨てて数を抑える
	constexpr size_t kMaxReceivedIds = 8;
	auto& ids = m_receivedAttackIds[attacker];
	ids.push_back(attackId);
	if (ids.size() > kMaxReceivedIds)
	{
		ids.erase(ids.begin());
	}
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
