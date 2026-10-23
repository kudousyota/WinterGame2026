#pragma once
#include <string>

//ステートパターンの基底クラス
//キャラクターごとの具体的なステートはこれを継承して作る
class StateBase
{
public:
	//ステートの種類
	enum class StateKind
	{
		Idle,//待機
		Walk,//移動
		Jump,//ジャンプ
		Dodge,//回避
		Death,//死亡
		Attack,//攻撃
		Damage,//被弾(のけぞり)
		Down,//叩き落とされて倒れる
		GetUp,//起き上がり
	};

	virtual ~StateBase() = default;

	//ステートに入った瞬間に一度だけ呼ばれる
	virtual void Enter() {}
	//毎フレーム呼ばれる
	virtual void Update() = 0;
	//ステートから抜ける瞬間に一度だけ呼ばれる
	virtual void Exit() {}

	//このステートの種類を返す
	virtual StateKind GetKind() const = 0;

#ifdef _DEBUG
	//デバッグ表示用のステート名を返す
	const char* GetKindName() const
	{
		switch (GetKind())
		{
		case StateKind::Idle:	return "Idle";
		case StateKind::Walk:	return "Walk";
		case StateKind::Jump:	return "Jump";
		case StateKind::Dodge:	return "Dodge";
		case StateKind::Attack:	return "Attack";
		case StateKind::Death:	return "Death";
		case StateKind::Damage:	return "Damage";
		case StateKind::Down:	return "Down";
		case StateKind::GetUp:	return "GetUp";
		default:				return "null";
		}
	}
	//ステートごとの詳しいデバッグ情報(攻撃のフレームなど)
	virtual std::string GetDebugInfo() const { return ""; }
#endif
};
