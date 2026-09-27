#pragma once
#include <memory>
#include "StateBase.h"
#include "../Player.h"

//プレイヤー用ステートの基底クラス
class PlayerStateBase : public StateBase
{
public:
	//プレイヤーを自動的に型変換しないようにする
	//引数で初期化
	explicit PlayerStateBase(Player& player) : m_player(player) {}

protected:
	//ステートを遷移する(切り替えはこのフレームのステート更新後に行われる)
	//テンプレートだと関数一個で済む、if文対策
	template<class Type>
	void ChangeState()
	{
		m_player.ChangeState(std::make_unique<Type>(m_player));
	}

	//左スティックの入力をXZ平面の移動方向として取得する
	Vector3 GetMoveInput() const;
	//入力に合わせて水平方向の速度を設定する(Y方向の速度は維持)
	void Move(float speedScale);
	//水平方向の速度を0にする(Y方向の速度は維持)
	void StopMove();

protected:
	//持ち主のプレイヤー(ステートはプレイヤーが所有して参照で持つ)
	Player& m_player;
};
