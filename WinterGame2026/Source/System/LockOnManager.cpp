#include "LockOnManager.h"
#include "../Game/Character/Character.h"
#include <cfloat>
#include <cmath>
#include "DebugDraw.h"

namespace
{
	//ロックオンの有効距離
	constexpr float kMaxDistance = 1000.0f;
	//この距離より近い敵は狙える
	constexpr float kNearDistance = 200.0f;
	//狙える角度
	constexpr float kMinDot = 0.7f;
	//近い敵は真横まで(cos90度=0)
	constexpr float kNearMinDot = 0.0f;
	//ロックオンは、近い敵なら後ろにいても狙える(cos180度=-1)
	//前の敵の方が点数が高いので、後ろの敵は前に狙える敵がいないときだけ選ばれる
	constexpr float kLockOnNearMinDot = -1.0f;
	//近い敵をどれくらい優先するか
	constexpr float kDistanceWeight = 0.3f;
	//長さがほぼ0のベクトルは正規化できないので除く
	constexpr float kMinSqLength = 0.0001f;
	//開始距離と同じにすると境界でついたり外れたりするので、少し長くする
	constexpr float kLoseDistance = 1300.0f;
	//ロックオンしていないときに攻撃で向く距離
	//遠い敵に向いてその場で空振りしないよう、近い敵だけにする
	constexpr float kSoftMaxDistance = 350.0f;
	//スティックを倒していないときは、近い敵なら後ろにいても振り向く(cos180度=-1)
	constexpr float kNoInputNearMinDot = -1.0f;
}

void LockOnManager::Start(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies)
{
	//見つからなかったらnullptrが返る
	m_pTarget = Search(playerPos, baseDir, enemies, kMaxDistance, kLockOnNearMinDot);
}

Character* LockOnManager::SearchSoft(const Vector3& playerPos, const Vector3& inputDir, const Vector3& forward, const std::vector<Character*>& enemies) const
{
	//スティックを倒していればその方向を優先する(倒した方向と逆の敵には振り向かない)
	if (inputDir.SqMagnitude() >= kMinSqLength)
	{
		return Search(playerPos, inputDir, enemies, kSoftMaxDistance, kNearMinDot);
	}
	//倒していなければ今向いている方向を基準にして、近い敵なら後ろにいても振り向く
	return Search(playerPos, forward, enemies, kSoftMaxDistance, kNoInputNearMinDot);
}

void LockOnManager::Release()
{
	m_pTarget = nullptr;
}

void LockOnManager::Update(const Vector3& playerPos)
{
	if (!m_pTarget)return;

	//死んだら外す
	if (m_pTarget->IsDead())
	{
		Release();
		return;
	}

	//距離が遠すぎる敵は外す
	Vector3 toTarget = m_pTarget->GetCenterPos() - playerPos;
	toTarget.y = 0.0f;
	if (toTarget.SqMagnitude() > kLoseDistance * kLoseDistance)
	{
		Release();
	}
}

std::optional<Vector3> LockOnManager::GetTargetPos() const
{
	if (!m_pTarget)
	{
		return std::nullopt;
	}
	return m_pTarget->GetCenterPos();
}

Character* LockOnManager::Search(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies, float maxDistance, float nearMinDot) const
{
	//基準の向きは正規化しておく
	Vector3 baseDirNorm = baseDir;
	baseDirNorm.y = 0.0f;
	if (baseDirNorm.SqMagnitude() < kMinSqLength)
	{
		return nullptr;
	}
	baseDirNorm = baseDirNorm.Normalize();

	Character* bestTarget = nullptr;
	float bestScore = -FLT_MAX;

	for (Character* enemy : enemies)
	{
		//死亡中は無視
		if (enemy->IsDead())continue;
		//敵への向き
		Vector3 toEnemy = enemy->GetCenterPos() - playerPos; 
		toEnemy.y = 0.0f;

		//距離が遠すぎる敵は無視
		float sqDistance = toEnemy.SqMagnitude();
		if (sqDistance > maxDistance * maxDistance)continue;
		//重なっている敵は無視
		if (sqDistance < kMinSqLength)continue;
		float distance = sqrtf(sqDistance);
		//正面に近い敵ほど点数が高くなるようにする
		float dot = baseDirNorm.Dot(toEnemy / distance);
		//近い敵は横にいても狙えるようにする
		float minDot = (distance < kNearDistance) ? nearMinDot : kMinDot;
		if (dot < minDot)continue;
		//距離が近いほど点数が高くなるようにする(探す距離に対する割合で減点する)
		float score = dot - (distance / maxDistance) * kDistanceWeight;
		if (score > bestScore)
		{
			bestScore = score;
			bestTarget = enemy;
		}
	}
	return bestTarget;
}

#ifdef _DEBUG
void LockOnManager::DrawDebug(const Vector3& playerPos, const Vector3& baseDir) const
{
	Vector3 dir = baseDir;
	dir.y = 0.0f;
	if (dir.SqMagnitude() < kMinSqLength) return;
	dir = dir.Normalize();

	//地面と重なってちらつかないよう少し浮かせる
	Vector3 center = playerPos + Vector3(0.0f, 2.0f, 0.0f);

	//遠い敵を狙える範囲(緑)
	DebugDraw::DrawFan(center, dir, kNearDistance, kMaxDistance, acosf(kMinDot), 0x00ff00);
	//近い敵を狙える範囲(黄)
	DebugDraw::DrawFan(center, dir, 0.0f, kNearDistance, acosf(kLockOnNearMinDot), 0xffff00);
	//ロックオンが外れる距離(赤)
	DebugDraw::DrawCircle(center, kLoseDistance, 0xff0000);
}
#endif