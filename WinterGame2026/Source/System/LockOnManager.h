#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "../Math/Vector3.h"
class Character;
class LockOnManager
{
public:
	//ロックオンを始める
	void Start(const Vector3& playerPos,const Vector3& baseDir,const std::vector<std::shared_ptr<Character>>& enemies);
	//ロックオンを終了する
	void Release();

	//狙っている敵がまだ狙えるか調べて、狙えなければ外す
	void Update(const Vector3& playerPos);
	//狙っている敵(いない、またはもう消えていれば空を返す)
	//返したshared_ptrはその場で使うだけにして、メンバに持たない
	std::shared_ptr<const Character> GetTarget() const { return m_target.lock(); }
	//狙っている位置(狙っていなければ何も入っていない)
	std::optional<Vector3> GetTargetPos() const;
	//ロックオンしていないときに攻撃で向く敵を取得する(近い敵だけ。状態は変えない)
	//inputDir:スティックの方向(倒していなければ長さ0) forward:今向いている方向
	std::shared_ptr<Character> SearchSoft(const Vector3& playerPos, const Vector3& inputDir, const Vector3& forward, const std::vector<std::shared_ptr<Character>>& enemies)const;

#ifdef _DEBUG
	//ロックオンできる範囲を足元に描く(baseDir:探すときの基準の向き)
	void DrawDebug(const Vector3& playerPos, const Vector3& baseDir) const;
#endif

private:
	//maxDistance以内で一番点数の高い敵を取得する
	//nearMinDot:近い敵を狙える角度(内積)
	std::shared_ptr<Character> Search(const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies, float maxDistance, float nearMinDot)const;
private:
	//狙っている敵(所有しない)
	//敵が消えるとlock()が空を返すので、消えた敵を触ってしまうことがない
	std::weak_ptr<Character> m_target;
};

