#pragma once
#include <vector>
#include <utility>
#include "CollisionManager.h"
#include "../DataLoader/AttackData.h"
#include "../Game/Character/Character.h"

class ProjectileManager;

struct BattleEvent
{
	enum class Type
	{
		//攻撃が当たった
		Hit,
		//ジャスト回避が成立した
		JustDodge,
		//ジャスト回避ではない普通の回避で攻撃をよけた
		Dodge,
	};

	Type type = Type::Hit;
	//攻撃した側//飛び道具のときはnullptr
	Character* attacker = nullptr;
	//攻撃された側
	Character* target = nullptr;
	//どの攻撃か
	AttackType attackType = AttackType{};
	//カメラシェイクの強さとフレーム数//フレーム数が0なら揺らさない
	//飛び道具は連射で何度も当たるので0にしている
	float shakePower = 0.0f;
	int shakeFrame = 0;
	//攻撃された側がどうなったか//Hitのときだけ使う
	HitResult result = HitResult::Ignored;
	//多段ヒットの何段目か//Hitのときだけ使う//飛び道具は1
	//イベントは値を順番に並べて作っているので、増やすときは一番下に足す
	int hitIndex = 1;
	//当たった位置//Hitのときだけ使う//攻撃された側の体の表面の、攻撃に一番近いところ
	Vector3 hitPos;
	//弱点に当たったか//Hitのときだけ使う//ダメージの倍率が1より大きければ弱点
	bool isWeakPoint = false;
};

class BattleSystem
{
public:
	BattleSystem(CollisionManager& collision, ProjectileManager& projectiles);

	//コピー禁止
	BattleSystem(const BattleSystem&) = delete;
	BattleSystem& operator=(const BattleSystem&) = delete;

	//全員の位置が確定してから呼ぶ
	//charactersはプレイヤーと敵の全員//このフレームの中だけで使う
	void Update(const std::vector<Character*>& characters);

	//このフレームで起きたことを受け取る//受け取ったら中身は空になる
	//キャラクターのポインタは敵が消える前のこのフレームの中だけで使う
	std::vector<BattleEvent> TakeEvents();

private:
	//申請された攻撃をまとめて判定し当たった相手にダメージを与える
	void ProcessAttacks(const std::vector<Character*>& characters);
	//申請された飛び道具を作る
	void ProcessShots(const std::vector<Character*>& characters);
	//飛び道具の当たり判定を行い当たった相手にダメージを与える
	void ProcessProjectileHits();
	//攻撃が当たったときの処理
	void ApplyAttackHit(Character& attacker, const AttackRequest& request, const CollisionManager::AttackHit& hit, const Vector3& start, const Vector3& end);
	//当たらなかったとき、回避でよけたなら1回だけ知らせる
	//attackerは飛び道具のときnullptr
	void CheckDodge(Character* attacker, Character& target, int attackId, AttackType attackType);

	//借りているだけなので消さない
	CollisionManager& m_collision;
	ProjectileManager& m_projectiles;

	//回避でよけた攻撃された側と攻撃番号の組
	//重なっている間ずっと数えないように覚えておく
	//攻撃番号は二度と出ないので、敵が消えて同じアドレスに別の敵ができても間違えない
	std::vector<std::pair<const Character*, int>> m_dodgedAttacks;

	//このフレームで起きたこと
	std::vector<BattleEvent> m_events;
};
