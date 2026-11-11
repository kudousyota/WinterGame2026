#include "LockOnManager.h"
#include "../Game/Character/Character.h"
#include <cfloat>
#include <cmath>
#include "DebugDraw.h"
#include "Input.h"
#include "../Effect/EffectManager.h"

namespace
{
	//ロックオンの有効距離
	constexpr float kMaxDistance = 1000.0f;
	//この距離より近い敵は狙える
	constexpr float kNearDistance = 200.0f;
	//狙える角度
	constexpr float kMinDot = 0.5f;
	//近い敵は真横まで
	constexpr float kNearMinDot = 0.0f;
	//ロックオンは、近い敵なら後ろにいても狙える
	//前の敵の方が点数が高いので、後ろの敵は前に狙える敵がいないときだけ選ばれる
	constexpr float kLockOnNearMinDot = -1.0f;
	//近い敵をどれくらい優先するか
	constexpr float kDistanceWeight = 1.2f;
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

void LockOnManager::Start(const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies)
{
	//見つからなかったら空のまま
	m_target = Search(playerPos, baseDir, enemies, kMaxDistance, kLockOnNearMinDot);
}

std::shared_ptr<Character> LockOnManager::SearchSoft(const Vector3& playerPos, const Vector3& inputDir, const Vector3& forward, const std::vector<std::shared_ptr<Character>>& enemies) const
{
	//スティックを倒していればその方向を優先する
	if (inputDir.SqMagnitude() >= kMinSqLength)
	{
		return Search(playerPos, inputDir, enemies, kSoftMaxDistance, kNearMinDot);
	}
	//倒していなければ今向いている方向を基準にして、近い敵なら後ろにいても振り向く
	return Search(playerPos, forward, enemies, kSoftMaxDistance, kNoInputNearMinDot);
}

void LockOnManager::Release()
{
	m_target.reset();
}

void LockOnManager::Update(const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies)
{
	//押したらロックオンを開始する
	if (Input::Instance().IsTriggered("target"))
	{
		//誰か狙っていたら外す
		if (m_target.lock())
		{
			Release();
		}
		//まだ誰も狙っていなければ探す
		else
		{
			Start(playerPos, baseDir, enemies);
		}
	}
	//狙っている敵が死んだら次の敵に移る
	auto target = m_target.lock();
	if(target && target->IsDead())
	{
		Retarget(playerPos, baseDir, enemies);
	}
	//死亡と距離のチェック
	CheckTarget(playerPos);

	//ロックオン中だけ、右スティック(またはE・Qキー)で対象を切り替える
	if (!m_target.lock())return;
	auto& input = Input::Instance();
	if (input.IsTriggered("targetRight"))
	{
		Switch(1, playerPos, baseDir, enemies);
	}
	if (input.IsTriggered("targetLeft"))
	{
		Switch(-1, playerPos, baseDir, enemies);
	}
}

std::shared_ptr<const Character> LockOnManager::GetAttackTarget(const Vector3& playerPos, const Vector3& inputDir, const Vector3& forward, const std::vector<std::shared_ptr<Character>>& enemies) const
{
	//ロックオン中はその敵
	if (auto target = m_target.lock())
	{
		return target;
	}
	//ロックオンしていないときは、近くで一番狙いやすい敵
	return SearchSoft(playerPos, inputDir, forward, enemies);
}

void LockOnManager::CheckTarget(const Vector3& playerPos)
{
	//敵が消えていたら空になる
	auto target = m_target.lock();
	if (!target)
	{
		Release();
		return;
	}

	//死んだら外す
	if (target->IsDead())
	{
		Release();
		return;
	}

	//距離が遠すぎる敵は外す
	Vector3 toTarget = target->GetCenterPos() - playerPos;
	toTarget.y = 0.0f;
	if (toTarget.SqMagnitude() > kLoseDistance * kLoseDistance)
	{
		Release();
	}
}

void LockOnManager::LateUpdate()
{
	auto& effect = EffectManager::Instns();
	auto target = m_target.lock();

	//狙っていなければマーカーを消す
	if (!target)
	{
		effect.FadeOutEffect(m_markerHandle);
		return;
	}

	//狙い始めたらマーカーを出す
	if (m_markerHandle == -1)
	{
		m_markerHandle = effect.PlayEffect(EffectType::LockOn, target->GetCenterPos());
		return;
	}
	//敵が動くので毎フレーム付いていかせる
	effect.SetEffectPos(m_markerHandle, target->GetCenterPos());
}

std::optional<Vector3> LockOnManager::GetTargetPos() const
{
	auto target = m_target.lock();
	if (!target)
	{
		return std::nullopt;
	}
	return target->GetCenterPos();
}

std::shared_ptr<Character> LockOnManager::Search(const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies, float maxDistance, float nearMinDot) const
{
	//基準の向きは正規化しておく
	Vector3 baseDirNorm = baseDir;
	baseDirNorm.y = 0.0f;
	if (baseDirNorm.SqMagnitude() < kMinSqLength)
	{
		return nullptr;
	}
	baseDirNorm = baseDirNorm.Normalize();

	std::shared_ptr<Character> bestTarget;
	float bestScore = -FLT_MAX;

	for (const auto& enemy : enemies)
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
		//距離が近いほど点数が高くなるようにする,探す距離に対する割合で減点する
		float score = dot - (distance / maxDistance) * kDistanceWeight;
		if (score > bestScore)
		{
			bestScore = score;
			bestTarget = enemy;
		}
	}
	return bestTarget;
}

void LockOnManager::Switch(int dir, const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies)
{
	//今狙っている敵いなければ切り替えない
	auto current = m_target.lock();
	if (!current) return;

	//カメラの右方向
	//敵どうしの大小を比べる
	Vector3 right(baseDir.z, 0.0f, -baseDir.x);

	//今の敵の画面上の横の位置
	float curX = (current->GetCenterPos() - playerPos).Dot(right);

	//はじいた方向で一番近い隣の敵
	std::shared_ptr<Character> best;
	float bestDiff = FLT_MAX;

	for (const auto& enemy : enemies)
	{
		//今の敵と死亡中は無視
		if (enemy == current) continue;
		if (enemy->IsDead()) continue;

		//遠すぎる敵は無視
		Vector3 toEnemy = enemy->GetCenterPos() - playerPos;
		toEnemy.y = 0.0f;
		if (toEnemy.SqMagnitude() > kMaxDistance * kMaxDistance) continue;

		//今の敵からはじいた方向へどれだけ離れているか
		//dirを掛けるので右でも左でもはじいた側ならプラスになる
		float diff = (toEnemy.Dot(right) - curX) * dir;
		//逆側の敵は無視
		if (diff <= 0.0f) continue;

		//一番近い隣を選ぶ
		if (diff < bestDiff)
		{
			bestDiff = diff;
			best = enemy;
		}
	}

	//見つかったときだけ入れ替える
	if (best)
	{
		m_target = best;
	}
}

void LockOnManager::Retarget(const Vector3& playerPos, const Vector3& baseDir, const std::vector<std::shared_ptr<Character>>& enemies)
{
	//見つからなかったら空のまま
	m_target = Search(playerPos, baseDir, enemies, kMaxDistance, kNearMinDot);
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