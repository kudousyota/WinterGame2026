#include "WickedWeave.h"

WickedWeave::WickedWeave(const AttackTable::AttackInfo& info, int attackId, const SummonParam& param) :
	m_info(info),
	m_attackId(attackId),
	m_hitPos(param.hitPos)
{
	m_effectHandle = EffectManager::Instns().PlayEffect(param.effectType, param.effectPos, param.effectRot, param.effectScale);
}

void WickedWeave::Update()
{
	m_frame++;
}

bool WickedWeave::IsHitActive() const
{
	return m_info.hitStartFrame <= m_frame && m_frame < m_info.hitEndFrame;
}

AttackRequest WickedWeave::MakeAttackRequest() const
{
	AttackRequest request{AttackData(CharacterType::Player, m_info.type, m_info.damage, m_info.radius)};
	request.data.SetKnockback(m_info.knockback);
	request.data.SetCanKnockdown(m_info.canKnockDown);
	request.attackId = m_attackId;
	//プレイヤーの位置ではなく、呼び出したときに決めた位置で判定する
	request.useWorldPos = true;
	request.worldPos = m_hitPos;
	return request;
}

bool WickedWeave::IsDead() const
{
	return m_frame >= m_info.hitEndFrame;
}
