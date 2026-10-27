#pragma once
#include "PlayerStateBase.h"

//プレイヤーの被弾状態(のけぞり)
//被弾アニメーションが終わるまで動けず、終わったら入力に応じて待機か移動に戻る
//空中で当たったときは重力で落ちながらのけぞる
class PlayerStateDamage : public PlayerStateBase
{
public:
	//usingでplayerを受け取って親に渡す
	using PlayerStateBase::PlayerStateBase;

	void Enter() override;
	void Update() override;
	StateKind GetKind() const override { return StateKind::Damage; }
};