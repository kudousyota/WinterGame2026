#include "BattleSystem.h"
#include "ProjectileManager.h"
#include "Projectile.h"
#include "Timer.h"
#include "../Game/Character/Character.h"
#include "../Game/Damage.h"
#include "../Game/Bullet.h"
#include "../Game/Shockwave.h"
#include "../DataLoader/DataManager.h"
#include "../Math/Geometry.h"
#include <algorithm>

namespace
{
	//リクエストの種類に合わせて飛び道具を作る
	//種類が増えたらここに足す
	std::unique_ptr<Projectile> CreateProjectile(const ShotRequest& request, const Vector3& pos, const Vector3& forward)
	{
		switch (request.kind)
		{
		case ShotKind::Shockwave:
			return std::make_unique<Shockwave>(pos, request.speed, request.data);
		case ShotKind::Bullet:
		default:
			return std::make_unique<Bullet>(pos, forward, request.speed, request.data);
		}
	}

	//当たった位置を求める//ヒットのエフェクトを出す場所に使う
	//攻撃の線分の中で体の中心に一番近い点を探し、体の中心からそちらへ半径の分だけ出た所にする
	//体の中に埋もれず、攻撃が来た側の表面に出る
	Vector3 CalcHitPos(const Character& target, const Vector3& start, const Vector3& end)
	{
		const Vector3 center = target.GetCenterPos();
		Vector3 toAttack = Geometry::ClosestPointOnSegment(start, end, center) - center;
		const float radius = target.GetCollisionRadius();
		//半径より遠ければ表面まで縮める//近ければ攻撃の位置そのまま
		if (toAttack.SqMagnitude() > radius * radius)
		{
			toAttack = toAttack.Normalize() * radius;
		}
		return center + toAttack;
	}
}

BattleSystem::BattleSystem(CollisionManager& collision, ProjectileManager& projectiles) :
	m_collision(collision),
	m_projectiles(projectiles)
{
}

void BattleSystem::Update(const std::vector<Character*>& characters)
{
	//位置が確定してから攻撃判定を行う
	ProcessAttacks(characters);
	ProcessShots(characters);
	//飛び道具の更新
	m_projectiles.Update();
	//飛び道具が動き終わってから当たり判定を行う
	ProcessProjectileHits();
}

std::vector<BattleEvent> BattleSystem::TakeEvents()
{
	//中身を渡して、こちらは空にする
	std::vector<BattleEvent> events;
	events.swap(m_events);
	return events;
}

void BattleSystem::ProcessAttacks(const std::vector<Character*>& characters)
{
	for (Character* attacker : characters)
	{
		for (const auto& request : attacker->TakeAttackRequests())
		{
			Vector3 pos = attacker->GetAttackPos(request);

			//lengthがあれば狙った方向へ伸びるカプセルなければ球で判定する
			std::vector<CollisionManager::AttackHit> hits;
			Vector3 end = pos;
			if (request.length > 0.0f)
			{
				end = pos + request.dir * request.length;
				hits = m_collision.CheckAttackCapsule(request.data, pos, end);
			}
			else
			{
				hits = m_collision.CheckAttackSphere(request.data, pos);
			}

			for (const auto& hit : hits)
			{
				ApplyAttackHit(*attacker, request, hit, pos, end);
			}
		}
	}
}

void BattleSystem::ProcessShots(const std::vector<Character*>& characters)
{
	for (Character* attacker : characters)
	{
		for (const auto& request : attacker->TakeShotRequests())
		{
			//確定した位置から判定の位置を求める
			//forwardOffsetとheightが0なら足元になる
			Vector3 pos = attacker->GetShotPos(request);

			Projectile* projectile = m_projectiles.Add(CreateProjectile(request, pos, attacker->GetForward()));
			//近接攻撃と同じように番号を付けて、回避を数えるのに使う
			projectile->SetAttackId(attacker->IssueAttackId());
		}
	}
}

void BattleSystem::ProcessProjectileHits()
{
	for (const auto& projectile : m_projectiles.GetProjectiles())
	{
		//寿命などですでに消えている飛び道具は判定しない
		if (projectile->IsDead()) continue;

		const AttackData& data = projectile->GetAttackData();

		//どんな形で当たるかは飛び道具ごとに決める
		for (const auto& hit : projectile->CheckHits(m_collision))
		{
			Character& target = *hit.target;

			//ジャスト回避は無敵中でも成立する
			if (hit.isJustDodge)
			{
				target.OnJustDodge();
				m_events.push_back({ BattleEvent::Type::JustDodge, nullptr, &target, data.GetAttackType() });
				//消えるか残るかは飛び道具が決める
				projectile->OnHitTarget(true);
				break;
			}

			//無敵中と死亡中は受けない//弾は消さずに通り抜ける
			if (!target.CanBeHit())
			{
				CheckDodge(nullptr, target, projectile->GetAttackId(), data.GetAttackType());
				continue;
			}

			//連射で何度も当たるので、ヒットストップとカメラシェイクは入れない
			HitResult result = target.OnHit(data, data.GetDamage());
			const Vector3 hitPos = CalcHitPos(target, projectile->GetPos(), projectile->GetPos());
			m_events.push_back({ BattleEvent::Type::Hit, nullptr, &target, data.GetAttackType(), 0.0f, 0, result, 1, hitPos });
			projectile->OnHitTarget(false);
			break;
		}
	}
}

void BattleSystem::ApplyAttackHit(Character& attacker, const AttackRequest& request, const CollisionManager::AttackHit& hit, const Vector3& start, const Vector3& end)
{
	Character& target = *hit.target;

	//同じ攻撃は1回しか受けない
	if (target.HasReceivedAttack(&attacker, request.attackId)) return;

	//ジャスト回避は無敵中でも成立する
	if (hit.isJustDodge)
	{
		target.MarkAttackReceived(&attacker, request.attackId);
		target.OnJustDodge();
		m_events.push_back({ BattleEvent::Type::JustDodge, &attacker, &target, request.data.GetAttackType() });
		return;
	}

	//無敵中、死亡中は受けない
	//記録しないから無敵が切れた後にまだ判定が出ていれば当たる
	if (!target.CanBeHit())
	{
		CheckDodge(&attacker, target, request.attackId, request.data.GetAttackType());
		return;
	}

	target.MarkAttackReceived(&attacker, request.attackId);
	HitResult result = target.OnHit(request.data, Damage::Calc(request.data, attacker, target, start, end));

	//ヒットストップはゲームの時間を止めるので、ここで頼む
	//カメラシェイクは見た目なので、イベントで知らせてシーンに任せる
	//強さは攻撃ごとに変える//多段ヒットは軽く、締めの技は重くできる
	const auto& info = DataManager::GetInstance().GetAttackTable().GetInfo(request.data.GetAttackType());
	if (info.hitStopFrame > 0)
	{
		Timer::Instance().RequestHitStop(info.hitStopFrame);
	}
	const Vector3 hitPos = CalcHitPos(target, start, end);
	//ダメージの計算と同じ倍率を見る//倍率が上がる場所に当たったら弱点
	const bool isWeakPoint = target.GetDamageRate(start, end, request.data.GetRadius()) > 1.0f;
	m_events.push_back({ BattleEvent::Type::Hit, &attacker, &target, request.data.GetAttackType(), info.shakePower, info.shakeFrame, result, request.hitIndex, hitPos, isWeakPoint });
}

void BattleSystem::CheckDodge(Character* attacker, Character& target, int attackId, AttackType attackType)
{
	//無敵になるのは今は回避中だけなので、無敵なら回避でよけたことになる
	//被弾後の無敵や死亡中は数えない
	if (!target.IsInvincible()) return;

	//同じ攻撃はもう数えた
	const std::pair<const Character*, int> key(&target, attackId);
	if (std::find(m_dodgedAttacks.begin(), m_dodgedAttacks.end(), key) != m_dodgedAttacks.end()) return;

	//同時に出ている攻撃は数個なので、古い記録は捨てて数を抑える
	constexpr size_t kMaxDodgedAttacks = 32;
	m_dodgedAttacks.push_back(key);
	if (m_dodgedAttacks.size() > kMaxDodgedAttacks)
	{
		m_dodgedAttacks.erase(m_dodgedAttacks.begin());
	}

	m_events.push_back({ BattleEvent::Type::Dodge, attacker, &target, attackType });
}
