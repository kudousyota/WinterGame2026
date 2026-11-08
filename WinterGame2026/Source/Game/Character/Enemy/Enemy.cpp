#include "Enemy.h"
#include "../../../System/Timer.h"
#include "State/EnemyStateDamage.h"
#include "State/EnemyStateDeath.h"
#include "State/EnemyStateKnockdown.h"
#include "State/EnemyStateAttack.h"
#include <cfloat>
#include "DxLib.h"
#include "EnemyManager.h"
#ifdef _DEBUG
#include "../../../System/DebugDraw.h"

namespace
{
	//攻撃の枠の目印を、頭のてっぺんからどれだけ上に出すか
	constexpr float kTokenMarkOffset = 40.0f;
	//攻撃の枠の目印の球の半径
	constexpr float kTokenMarkRadius = 20.0f;
}
#endif

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
	//運ぶ攻撃なら、のけぞっている間は攻撃してきた相手の目の前についていく
	ChangeState(std::make_unique<EnemyStateDamage>(*this, attackData.GetKnockback(), attackData.IsCarry()));
}

void Enemy::CheckDeath()
{
	if (m_hp > 0 || m_isDead) return;

	//死亡中は攻撃を受けない(CanBeHitがfalseになる)
	m_isDead = true;
	ChangeState(std::make_unique<EnemyStateDeath>(*this));
}

void Enemy::DecideAttack()
{
	m_attackType = SelectAttack();
}

bool Enemy::CanStartAttack(float distance) const
{
	return distance <= m_attackRange;
}

std::unique_ptr<StateBase> Enemy::CreateAttackState()
{
	return std::make_unique<EnemyStateAttack>(*this);
}

AttackType Enemy::SelectAttack()
{
	//攻撃が1種類の敵は、Initで決めた攻撃をそのまま使う
	return m_attackType;
}

void Enemy::OnAttackFinished()
{
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

#ifdef _DEBUG
Vector3 Enemy::GetDebugGroundCenter() const
{
	//距離はXZ平面で測っているので、ターゲット(プレイヤー)の足元の高さに描く
	//(敵は浮いているので、敵の高さに描くと地面から離れて見づらい)
	//ジャンプ中に円がついてこないように、高さはプレイヤーが着地している時だけ更新する
	//(最初の1回はまだ高さがないので、空中でもそのまま使う)
	if (m_pTarget && (m_pTarget->IsGround() || !m_hasDebugGroundY))
	{
		m_debugGroundY = m_pTarget->GetPos().y;
		m_hasDebugGroundY = true;
	}
	Vector3 center = m_pos;
	if (m_hasDebugGroundY) center.y = m_debugGroundY;
	center.y += 2.0f;
	return center;
}

void Enemy::DrawDebugRange() const
{
	const Vector3 center = GetDebugGroundCenter();

	//索敵範囲(水色):この中にプレイヤーが入ると追いかける
	DebugDraw::DrawCircle(center, m_searchRange, 0x00ffff);
	//攻撃範囲(オレンジ):この中なら攻撃する
	DebugDraw::DrawCircle(center, m_attackRange, 0xff8000);
	//攻撃の後に離れる距離(白)
	DebugDraw::DrawCircle(center, m_keepDistance, 0xffffff);
	//近すぎる距離(ピンク):この中に入られると離れる
	DebugDraw::DrawCircle(center, m_tooCloseDistance, 0xff00ff);
}

void Enemy::DrawDebugForward() const
{
	const Vector3 center = GetDebugGroundCenter();
	//向いている方向(視線)
	DrawLine3D(center, center + m_forward * m_tooCloseDistance, 0xff00ff);
}

void Enemy::DrawDebugToken() const
{
	//攻撃の枠を持っているかを頭の上の球で表示する
	//赤く塗りつぶした大きい球:枠を持っている(攻撃できる)
	//灰色の線だけの小さい球:枠を持っていない(順番待ち)
	const Vector3 headPos = m_pos + Vector3(0.0f, m_height + kTokenMarkOffset, 0.0f);
	if (m_hasAttackToken)
	{
		DrawSphere3D(headPos, kTokenMarkRadius, 12, 0xff0000, 0xff0000, true);
	}
	else
	{
		DrawSphere3D(headPos, kTokenMarkRadius * 0.5f, 8, 0x808080, 0x808080, false);
	}
}
#endif

bool Enemy::TryTakeAttackToken()
{
	//マネージャーがいないときは、いつでも攻撃していい
	if (!m_pManager) return true;
	//もう持っているならそのまま攻撃していい
	if (m_hasAttackToken) return true;

	m_hasAttackToken = m_pManager->TryTakeAttackToken();
	return m_hasAttackToken;
}

void Enemy::ReleaseAttackToken()
{
	//持っていないのに返すと、ほかの敵の枠を減らしてしまう
	if (!m_hasAttackToken) return;

	m_pManager->ReleaseAttackToken();
	m_hasAttackToken = false;
}