#include "Enemy.h"
#include "../../../System/Timer.h"
#include "State/EnemyStateDamage.h"
#include "State/EnemyStateDeath.h"
#include "State/EnemyStateKnockdown.h"
#include <cfloat>

Enemy::Enemy()
{
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{

}

void Enemy::Update()
{

}

void Enemy::Draw()
{

}

void Enemy::OnHit(const AttackData& attackData, int damage)
{
	//無敵中・死亡中は当たっていないのでのけぞらない
	if (!CanBeHit()) return;

	Character::OnHit(attackData, damage);

	//このダメージで倒れた時はのけぞらない(死亡はUpdateで処理する)
	if (m_hp <= 0) return;

	//このフレームにもう予約されているステートがあればそちらで判断する
	//(同じフレームに叩き落としと通常の攻撃が当たっても、叩き落としが消えないようにする)
	const StateBase* state = m_pNextState ? m_pNextState.get() : m_pState.get();
	if (state)
	{
		auto kind = state->GetKind();
		//倒れている間・起き上がり中はダメージだけ受ける(追い打ちはできる)
		if (kind == StateBase::StateKind::Down || kind == StateBase::StateKind::GetUp) return;
	}

	//叩き落とせる攻撃は、浮いている敵を地面に落とす
	if (attackData.CanKnockdown() && !m_knockdownAnimName.empty())
	{
		ChangeState(std::make_unique<EnemyStateKnockdown>(*this, attackData.GetKnockback()));
		return;
	}

	//攻撃中でも割り込んでのけぞる。のけぞり中に当たったら最初からのけぞり直す
	//吹っ飛ぶ攻撃(knockbackが0より大きい)なら、のけぞりながら後ろへ下がる
	if (m_damageAnimName.empty()) return;
	ChangeState(std::make_unique<EnemyStateDamage>(*this, attackData.GetKnockback()));
}

void Enemy::CheckDeath()
{
	if (m_hp > 0 || m_isDead) return;

	//死亡中は攻撃を受けない(CanBeHitがfalseになる)
	m_isDead = true;
	ChangeState(std::make_unique<EnemyStateDeath>(*this));
}

float Enemy::GetTargetDistance() const
{
	if (!m_pTarget) return FLT_MAX;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	return toTarget.Magnitude();
}

void Enemy::MoveToTarget(float speedScale)
{
	if (!m_pTarget) return;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	//ほぼ重なっている時は向きが決まらないので止まる
	if (toTarget.SqMagnitude() <= 0.0f)
	{
		StopMove();
		return;
	}

	Vector3 move = toTarget.Normalize() * (m_speed * speedScale);
	m_velocity = Vector3(move.x, m_velocity.y, move.z);
}

void Enemy::FaceTarget()
{
	if (!m_pTarget) return;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	//ほぼ重なっている時は向きが決まらないので今の向きのまま
	if (toTarget.SqMagnitude() <= 0.0f) return;

	m_forward = toTarget.Normalize();
}

float Enemy::GetTimeScale() const
{
	return Timer::Instance().GetTimeScale() * Timer::Instance().GetEnemyTimeScale();
}

void Enemy::MoveAwayFromTarget(float speedScale)
{
	if (!m_pTarget)return;

	Vector3 away = m_pos - m_pTarget->GetPos();
	away.y = 0.0f;
	//重なった時は離れる向きは後ろにする
	if (away.SqMagnitude() <= 0.0f)
	{
		away = m_forward * -1.0f;
	}

	Vector3 move = away.Normalize() * (m_speed * speedScale);
	m_velocity = Vector3(move.x, m_velocity.y, move.z);

}

void Enemy::StrafeAroundTarget(float speedScale, float side)
{
	if (!m_pTarget) return;

	Vector3 toTarget = m_pTarget->GetPos() - m_pos;
	toTarget.y = 0.0f;
	if (toTarget.SqMagnitude() <= 0.0f)
	{
		StopMove();
		return;
	}

	//ターゲットへの方向と上方向の外積で横の方向を求める
	Vector3 sideDir = toTarget.Normalize().Cross(Vector3(0.0f, 1.0f, 0.0f)) * side;
	Vector3 move = sideDir * (m_speed * speedScale);
	m_velocity = Vector3(move.x, m_velocity.y, move.z);
}