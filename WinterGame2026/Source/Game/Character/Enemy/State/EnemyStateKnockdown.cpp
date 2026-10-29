#include "EnemyStateKnockdown.h"
#include "EnemyStateGetUp.h"
#include <algorithm>
#include <cstdio>

namespace
{
	//吹っ飛びのアニメーション(FlyingBack・全92フレーム)で、体が宙に浮いて後ろへ飛んでいる区間(アニメーション全体に対する割合)
	//当たった瞬間から飛び始めて、27フレーム目で背中から地面に着く
	//この区間の外(倒れた後)は動かさないので、体が滑って見えない
	constexpr float kFlyStartRate = 0.0f;
	constexpr float kFlyEndRate = 0.29f;
	//地面に倒れきった割合(着地した後の揺れが収まるまで流してから止める)
	constexpr float kLieDownRate = 0.9f;

	//飛び始めが一番速く、着地に向けてゆっくりになる(アニメーションの腰の動きに合わせる)
	float EaseOut(float t)
	{
		return 1.0f - (1.0f - t) * (1.0f - t);
	}
	//倒れたままでいるフレーム数(この間に地上のコンボを入れられる)
	constexpr float kLieFrame = 90.0f;
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
	//地面(y=0)まで落ちる高さを覚えておく
	//地上で浮いている敵なら浮いている高さ(90cm)、運ばれて高い所にいればその分だけ多く落ちる
	m_fallHeight = (std::max)(m_enemy.GetPos().y, 0.0f);

}

void EnemyStateKnockdown::Update()
{
	//倒れきったあとは倒れたまま
	if (m_enemy.IsAnimPause())
	{
		m_lieFrame += m_enemy.GetTimeScale();
		if (m_lieFrame >= kLieFrame)
		{
			ChangeState<EnemyStateGetUp>();
		}
		return;
	}

	m_enemy.StopMove();
	float animRate = m_enemy.GetAnim().GetAnimRate();
	//宙に浮いている区間でのラープ
	float t = (animRate - kFlyStartRate) / (kFlyEndRate - kFlyStartRate);
	//区間の外は0か１にする
	t = std::clamp(t, 0.0f, 1.0f);

	//このフレームで進む距離
	float move = (EaseOut(t) - EaseOut(m_prevT)) * m_distance;
	//このフレームで下がる高さ
	float fall = (t - m_prevT) * m_fallHeight;
	//後ろへずらして下にも下げる
	Vector3 delta = m_flyDir * move + Vector3(0.0f, -fall, 0.0f);
	m_enemy.SetPosition(m_enemy.GetPos() + delta);
	//次のフレーム用に覚えておく
	m_prevT = t;

	//地面に倒れきったらアニメーションを止める
	if (animRate >= kLieDownRate)
	{
		m_enemy.SetAnimPause(true);
	}
}

void EnemyStateKnockdown::Exit()
{
	//倒れている途中で死亡したときも、アニメーションが止まったままにならないようにする
	m_enemy.SetAnimPause(false);
}

#ifdef _DEBUG
std::string EnemyStateKnockdown::GetDebugInfo() const
{
	char buf[96];
	sprintf_s(buf, "Distance:%.0f  Fall:%.0f  Anim:%.2f  Lie:%.0f", m_distance, m_fallHeight, m_enemy.GetAnim().GetAnimRate(), m_lieFrame);
	return buf;
}
#endif
