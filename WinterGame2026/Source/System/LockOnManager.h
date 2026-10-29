#pragma once
#include <optional>
#include <vector>
#include "../Math/Vector3.h"
class Character;
class LockOnManager
{
public:
	//ロックオンを始める
	void Start(const Vector3& playerPos,const Vector3& baseDir,const std::vector<Character*>& enemies);
	//ロックオンを終了する
	void Release();

	//狙っている敵がまだ狙えるか調べて、狙えなければ外す
	void Update(const Vector3& playerPos);
	//狙っている敵いなければnullptrを返す
	const Character* GetTarget() const { return m_pTarget; }
	//T型の値が入っているか、何も入っていないか//狙っている位置
	std::optional<Vector3> GetTargetPos() const;
	//ロックオンしていないときに攻撃で向く敵を取得する(近い敵だけ。状態は変えない)
	//inputDir:スティックの方向(倒していなければ長さ0) forward:今向いている方向
	Character* SearchSoft(const Vector3& playerPos, const Vector3& inputDir, const Vector3& forward, const std::vector<Character*>& enemies)const;

#ifdef _DEBUG
	//ロックオンできる範囲を足元に描く(baseDir:探すときの基準の向き)
	void DrawDebug(const Vector3& playerPos, const Vector3& baseDir) const;
#endif

private:
	//maxDistance以内で一番点数の高い敵を取得する
	//nearMinDot:近い敵を狙える角度(内積)
	Character* Search(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies, float maxDistance, float nearMinDot)const;
private:
	//所有しない
	Character* m_pTarget = nullptr;
};

