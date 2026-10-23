#include "EnemyStateKnockdown.h"
#include "EnemyStateGetUp.h"
#include <algorithm>
#include <cstdio>

namespace
{
	//吹っ飛びのアニメーション(HitBackward)で、体が宙に浮いて後ろへ飛んでいる区間(アニメーション全体に対する割合)
	//この区間の外(殴られて立っている間・倒れた後)は動かさないので、足が滑って見えない
	constexpr float kFlyStartRate = 0.21f;
	constexpr float kFlyEndRate = 0.55f;
	//地面に倒れきったら起き上がりに移る(HitBackwardはこの後、自分で起き上がりかけるので、そこで切り替える)
	constexpr float kLieDownRate = 0.55f;
}

EnemyStateKnockdown::EnemyStateKnockdown(Enemy& enemy, float distance) :
	EnemyStateBase(enemy),
	m_distance(distance)
{
}

void EnemyStateKnockdown::Enter()
{
	m_enemy.StopMove();
	//攻撃してきた相手の方を向いて、背中側へ倒れる
	m_enemy.FaceTarget();
	m_flyDir = m_enemy.GetForward() * -1.0f;
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetKnockdownAnimName(), false);

}

void EnemyStateKnockdown::Update()
{
	m_enemy.StopMove();
	float animRate = m_enemy.GetAnim().GetAnimRate();
	//宙に浮いている区間でのラープ
	float t = (animRate - kFlyStartRate) / (kFlyEndRate - kFlyStartRate);
	//区間の外は0か１にする
	t = std::clamp(t, 0.0f, 1.0f);

	//このフレームで進む距離
	float move = (t - m_prevT) * m_distance;
	//このフレームで下がる高さ
	float fall = (t - m_prevT) * m_enemy.GetHoverHeight();
	//後ろへずらして下にも下げる
	Vector3 delta = m_flyDir * move + Vector3(0.0f, -fall, 0.0f);
	m_enemy.SetPosition(m_enemy.GetPos() + delta);
	//次のフレーム用に覚えておく
	m_prevT = t;

	//地面に倒れきったら起き上がりへ
	if (animRate >= kLieDownRate)
	{
		ChangeState<EnemyStateGetUp>();
	}
}

#ifdef _DEBUG
std::string EnemyStateKnockdown::GetDebugInfo() const
{
	char buf[96];
	sprintf_s(buf, "Distance:%.0f  Anim:%.2f", m_distance, m_enemy.GetAnim().GetAnimRate());
	return buf;
}
#endif
