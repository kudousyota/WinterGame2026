#pragma once
#include <memory>
#include "../../StateBase.h"
#include "../Player.h"
#include "../../../../DataLoader/AttackData.h"

//プレイヤー用ステートの基底クラス
class PlayerStateBase : public StateBase
{
public:
	//プレイヤーを自動的に型変換しないようにする
	//引数で初期化
	explicit PlayerStateBase(Player& player) : m_player(player) {}

protected:
	//ステートを遷移する
	//テンプレートだと関数一個で済むif文対策
	template<class Type>
	void ChangeState()
	{
		m_player.ChangeState(std::make_unique<Type>(m_player));
	}
	//攻撃ステートに遷移する
	void ChangeAttackState(AttackType type, const char* buttonName);

	//左スティックの入力をXZ平面の移動方向として取得する
	Vector3 GetMoveInput() const;
	//入力に合わせて水平方向の速度を設定する
	void Move(float speedScale);
	//入力に向かって水平方向の速度を少しずつ近づける
	void MoveSmooth(float speedScale);
	//水平方向の速度を0にする
	void StopMove();

	//地上で出せる行動の入力を見てあればステートを切り替える
	bool TryGroundAction(int bufferFrame);

protected:
	//持ち主のプレイヤー
	Player& m_player;
};
