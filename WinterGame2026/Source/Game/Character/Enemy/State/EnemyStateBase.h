#pragma once
#include <memory>
#include "../../StateBase.h"
#include "../Enemy.h"

//敵用ステートの基底クラス
class EnemyStateBase : public StateBase
{
public:
	//敵を自動的に型変換しないようにする
	explicit EnemyStateBase(Enemy& enemy) : m_enemy(enemy) {}

protected:
	//ステートを遷移する(切り替えはこのフレームのステート更新後に行われる)
	template<class Type>
	void ChangeState()
	{
		m_enemy.ChangeState(std::make_unique<Type>(m_enemy));
	}

protected:
	//ステートは基底クラスが所有して参照で持つ
	Enemy& m_enemy;
};
