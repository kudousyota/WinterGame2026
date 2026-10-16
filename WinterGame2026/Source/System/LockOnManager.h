#pragma once
#include <optional>
#include <vector>
#include "../Math/Vector3.h"
class Character;
class LockOnManager
{
public:
	//狙う敵を取得する
	void Update(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies);
	//狙っている敵いなければnullptrを返す
	const Character* GetTarget() const { return m_pTarget; }
	//狙っている位置
	std::optional<Vector3> GetTargetPos() const;
private:
	//一番点数の高い敵を取得する
	Character* Search(const Vector3& playerPos,const Vector3& baseDir,const std::vector<Character*>& enemies)const;
private:
	//所有しない
	Character* m_pTarget = nullptr;
};

