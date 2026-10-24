#pragma once
#include "EnemyStateBase.h"

//敵が叩き落とされた状態
//攻撃してきた相手から離れる方向へ吹っ飛びながら、浮いている高さの分だけ地面へ落ちる
//地面に倒れたらしばらくそのまま(追い打ちの時間)、その後起き上がりに移る
//アニメーションは移動を消してあるので、宙に浮いている間だけコードで後ろへ動かす
class EnemyStateKnockdown : public EnemyStateBase
{
public:
	//distance:吹っ飛ぶ距離(Attack.csvのknockback)
	EnemyStateKnockdown(Enemy& enemy, float distance);

	void Enter() override;
	void Update() override;
	void Exit() override;
	StateKind GetKind() const override { return StateKind::Down; }
#ifdef _DEBUG
	std::string GetDebugInfo() const override;
#endif

private:
	//吹っ飛ぶ方向
	Vector3 m_flyDir = Vector3(0.0f,0.0f,0.0f);
	//吹っ飛ぶ距離
	float m_distance = 0.0f;
	//前のフレームまでに進んだ割合
	float m_prevT = 0.0f;
	//倒れきってからのフレーム数
	float m_lieFrame = 0.0f;
};
