#include "LockOnManager.h"
#include "../Game/Character/Character.h"

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
	//近い敵をどれくらい優先するか
	constexpr float kDistanceWeight = 0.3f;
	//長さがほぼ0のベクトルは正規化できないので除く
	constexpr float kMinSqLength = 0.0001f;
}

void LockOnManager::Update(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies)
{
	m_pTarget = Search(playerPos, baseDir, enemies);
}

std::optional<Vector3> LockOnManager::GetTargetPos() const
{
	if (!m_pTarget)
	{
		return std::nullopt;
	}
	return m_pTarget->GetCenterPos();
}

Character* LockOnManager::Search(const Vector3& playerPos, const Vector3& baseDir, const std::vector<Character*>& enemies) const
{
	//基準の向きは正規化しておく
	Vector3 baseDirNorm = baseDir.Normalize();
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
		if (!enemy->IsDead())continue;;
		//敵への向き
		Vector3 toEnemy = enemy->GetCenterPos() - playerPos; 
		toEnemy.y = 0.0f;

		//距離が遠すぎる敵は無視
		float sqDistance = toEnemy.SqMagnitude();
		if (sqDistance > kMaxDistance * kMaxDistance)continue;
		//重なっている敵は無視
		if (sqDistance < kMinSqLength)continue;
		float distance = sqrtf(sqDistance);
		//正面に近い敵ほど点数が高くなるようにする
		float dot = baseDirNorm.Dot(toEnemy / distance);
		//近い敵は横にいても狙えるようにする
		float minDot = (distance < kNearDistance) ? kNearMinDot : kMinDot;
		if (dot < minDot)continue;
		//距離が近いほど点数が高くなるようにする
		float score = dot - (distance / kMaxDistance) * kDistanceWeight;
		if (score > bestScore)
		{
			bestScore = score;
			bestTarget = enemy;
		}
	}
	return bestTarget;
}