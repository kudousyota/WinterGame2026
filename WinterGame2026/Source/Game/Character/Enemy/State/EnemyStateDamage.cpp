#include "EnemyStateDamage.h"
#include "EnemyStateIdle.h"
#include <algorithm>

namespace
{
	//のけぞりのアニメーションのこの割合までで下がりきる(当たった直後に勢いよく下がる)
	constexpr float kSlideEndRate = 0.4f;
	//運ばれるとき、1フレームで目標の位置までの残りの何割を詰めるか(1ならワープ、小さいほどゆっくりついていく)
	constexpr float kCarryRate = 0.3f;
	//運ばれるとき、体同士の間に空ける隙間(プレイヤーが敵の手前で止まるときと同じ)
	constexpr float kCarryMargin = 10.0f;
}

EnemyStateDamage::EnemyStateDamage(Enemy& enemy, float distance, bool isCarry) :
	EnemyStateBase(enemy),
	m_distance(distance),
	m_isCarry(isCarry)
{
}

void EnemyStateDamage::Enter()
{
	//のけぞり中はその場で止まる
	m_enemy.StopMove();
	//攻撃してきた相手の方を向いてのけぞる
	m_enemy.FaceTarget();
	//背中側へ下がる
	m_flyDir = m_enemy.GetForward() * -1.0f;
	m_enemy.GetAnim().ChangeAnim(m_enemy.GetDamageAnimName(), false);
}

void EnemyStateDamage::Update()
{
	m_enemy.StopMove();

	//吹っ飛ぶ攻撃なら、アニメーションの進み具合に合わせて後ろへ下がる
	if (m_distance > 0.0f)
	{
		float t = std::clamp(m_enemy.GetAnim().GetAnimRate() / kSlideEndRate, 0.0f, 1.0f);
		Vector3 delta = m_flyDir * ((t - m_prevT) * m_distance);
		m_enemy.SetPosition(m_enemy.GetPos() + delta);
		m_prevT = t;
	}

	//運ぶ攻撃なら、攻撃してきた相手の目の前についていく
	if (m_isCarry)
	{
		UpdateCarry();
	}

	//のけぞりのアニメーションが終わったら待機に戻る
	if (m_enemy.GetAnim().GetAnimEndFlag())
	{
		ChangeState<EnemyStateIdle>();
	}
}

void EnemyStateDamage::UpdateCarry()
{
	const Character* target = m_enemy.GetTarget();
	if (!target) return;

	//相手の前方、体同士が触れない距離の位置
	float distance = target->GetCollisionRadius() + m_enemy.GetCollisionRadius() + kCarryMargin;
	Vector3 goal = target->GetPos() + target->GetForward() * distance;
	//高さは「相手の足元＋いつも浮いている高さ」にする
	//地上と同じ位置関係のまま上がっていくので、相手の攻撃判定に入り続ける
	goal.y = target->GetPos().y + m_enemy.GetHoverHeight();

	//当たるたびにワープするとカクカクするので、毎フレーム少しずつ寄せる
	Vector3 pos = m_enemy.GetPos();
	m_enemy.SetPosition(pos + (goal - pos) * kCarryRate);
}