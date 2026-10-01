#pragma once
#include <vector>
#include "Vector3.h"
#include "../Game/Character/Character.h"
class Character;
class AttackData;
class CollisionManager
{
public:
	static CollisionManager& Instance();
	void Register(Character* character);
	void Unregister(Character* character);

	//攻撃用の球判定　//キャラクター事の攻撃当たり判定
	std::vector<Character*> CheckAttackSphere(const AttackData& attackdata, const Vector3& pos);
	//攻撃用のカプセル判定
	std::vector<Character*> CheckAttackCapsule(const AttackData& attackdata, const Vector3& start, const Vector3& end);
	//ステージとの判定
	bool CheckStageWall(Character* caracter, int stagehandle);
	//ステージの地面
	bool CheckStageGround(Character* character, int stagehandle, float& outGroundY, Vector3& outGroundNormal);
	//カメラとステージの判定
	bool CheckCameraRay(int stagehandle, const Vector3& start, const Vector3& end, Vector3& hitpos);

	void ResolveCharacterCollision(Character* characterA, Character* characterB);

	// 指定キャラクターと登録済みの他キャラクターとのカプセル衝突を解決する
	void ResolveCollisionsFor(Character* character);

	void ClosestPointSegmentSegment(const Vector3& p1, const Vector3& q1, const Vector3& p2, const Vector3& q2, Vector3& outC1, Vector3& outC2);

	bool CheckCharacterCapsule(Character* characterA, Character* characterB);
	//カプセルの当たり判定
	bool CheckCapsule(const Vector3& aStart, const Vector3& aEnd, float radiusA, const Vector3& bStart, const Vector3& bEnd, float radiusB);


private:
	std::vector<Character*> m_pCharacters;

};

