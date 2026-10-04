#pragma once
#include <vector>
#include "Vector3.h"
class Character;
class AttackData;

//登録されたキャラクター同士の当たり判定を管理する
//GameSceneが所有し、全員の更新が終わった後にまとめて判定する
class CollisionManager
{
public:
	//攻撃判定の結果
	struct AttackHit
	{
		//当たったキャラクター
		Character* target = nullptr;
		//ジャスト回避が成立したか(trueならダメージは与えない)
		bool isJustDodge = false;
	};

	void Register(Character* character);
	void Unregister(Character* character);

	//フレームの最初に呼ぶ(デバッグ表示用の記録を消す)
	void BeginFrame();

	//登録済みキャラクター同士の重なりをまとめて解消する
	void ResolveAll();

	//攻撃用の球判定
	//当たったかどうかを調べるだけで、ダメージなどの処理は呼び出し側で行う
	std::vector<AttackHit> CheckAttackSphere(const AttackData& attackdata, const Vector3& pos);
	//攻撃用のカプセル判定
	std::vector<AttackHit> CheckAttackCapsule(const AttackData& attackdata, const Vector3& start, const Vector3& end);

	//当たり判定を描画する(デバッグ用)
	void DrawDebug()const;

private:
	//2つのキャラクターのカプセルが重なっているか判定する
	bool CheckCharacterCapsule(Character* characterA, Character* characterB);
	//重なっている2つのキャラクターを押し出す
	void ResolveCharacterCollision(Character* characterA, Character* characterB);

private:
	//判定対象のキャラクター(所有はしない)
	std::vector<Character*> m_pCharacters;

	//デバッグ表示用に、このフレームに判定した攻撃の形を覚えておく
	struct DebugAttackShape
	{
		Vector3 start;
		Vector3 end;
		float radius;
	};
	std::vector<DebugAttackShape> m_debugAttackShapes;

};

