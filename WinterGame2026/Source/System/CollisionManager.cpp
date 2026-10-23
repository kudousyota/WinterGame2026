#include "CollisionManager.h"
#include "../Game/Character/Character.h"
#include <algorithm>
#include <vector>
#include "../DataLoader/AttackData.h"
#include "Geometry.h"

//当たり判定対象のキャラクターを登録
void CollisionManager::Register(Character* character)
{
	m_pCharacters.push_back(character);
}
//登録済みのキャラクターを削除
void CollisionManager::Unregister(Character* character)
{
	//登録済みキャラクターの中から対象を探す
	auto iterator = std::find(m_pCharacters.begin(), m_pCharacters.end(), character);
	//見つかったら登録リストから削除
	if (iterator != m_pCharacters.end())
	{
		m_pCharacters.erase(iterator);
	}
}

//フレームの最初に呼ぶ
void CollisionManager::BeginFrame()
{
	//前のフレームに判定した攻撃の形を消す
	m_debugAttackShapes.clear();
}

//登録済みキャラクター同士の重なりをまとめて解消する
void CollisionManager::ResolveAll()
{
	//同じ組み合わせを2回判定しないように、自分より後ろのキャラクターとだけ判定する
	for (size_t i = 0; i < m_pCharacters.size(); i++)
	{
		for (size_t j = i + 1; j < m_pCharacters.size(); j++)
		{
			Character* characterA = m_pCharacters[i];
			Character* characterB = m_pCharacters[j];

			//倒れているキャラクターは押し合わない(死体に引っかからないようにする)
			if (characterA->IsDead() || characterB->IsDead()) continue;

			//衝突していたら解決
			if (CheckCharacterCapsule(characterA, characterB))
			{
				ResolveCharacterCollision(characterA, characterB);
			}
		}
	}
}

//球状の攻撃判定を行う
std::vector<CollisionManager::AttackHit> CollisionManager::CheckAttackSphere(const AttackData& attackdata, const Vector3& pos)
{
	//デバッグ表示用に記録
	m_debugAttackShapes.push_back({ pos, pos, attackdata.GetRadius() });

	std::vector<AttackHit> hits;

	//登録済みの全キャラクターを探す
	for (auto& character : m_pCharacters)
	{
		//攻撃した側と同じタイプには当たらない
		if (character->GetCharacterType() == attackdata.GetAttacker())
		{
			continue;
		}

		//キャラクターのカプセル
		Geometry::Capsule capsule = character->GetCollisionCapsule();

		//カプセル軸上で攻撃位置に一番近い点
		Vector3 closest = Geometry::ClosestPointOnSegment(capsule.start, capsule.end, pos);
		//最近接点までの距離の2乗
		float dist2 = (closest - pos).SqMagnitude();
		//攻撃判定とジャスト回避判定合わせた範囲
		float combinedJust = attackdata.GetRadius() + character->GetJustDodgeRadius();

		if (dist2 <= combinedJust * combinedJust)
		{
			//もし相手がジャスト回避受付中なら、ジャスト回避成功
			if (character->IsJustDodgeWindow())
			{
				hits.push_back({ character, true });
				continue;
			}
		}

		//ジャスト回避が不発、または受付時間外だった場合、通常の判定でチェック
		float combinedNormal = attackdata.GetRadius() + capsule.radius;

		if (dist2 <= combinedNormal * combinedNormal)
		{
			hits.push_back({ character, false });
		}
	}
	return hits;
}
//線分と半径からなるカプセル型の攻撃判定を行う
std::vector<CollisionManager::AttackHit> CollisionManager::CheckAttackCapsule(const AttackData& attackdata, const Vector3& start, const Vector3& end)
{
	//デバッグ表示用に記録
	m_debugAttackShapes.push_back({ start, end, attackdata.GetRadius() });

	std::vector<AttackHit> hits;

	//登録済みの全キャラクターを探す
	for (auto& character : m_pCharacters)
	{
		//攻撃した側と同じタイプには当たらない
		if (character->GetCharacterType() == attackdata.GetAttacker())
		{
			continue;
		}
		//相手キャラクターのカプセル
		Geometry::Capsule capsule = character->GetCollisionCapsule();

		//攻撃カプセルの軸と相手カプセルの軸との最短距離を求める
		float distance = Segment_Segment_MinLength(start, end, capsule.start, capsule.end);

		//攻撃判定とジャスト回避判定合わせた範囲
		float justRange = attackdata.GetRadius() + character->GetJustDodgeRadius();

		if (distance <= justRange)
		{
			if (character->IsJustDodgeWindow())
			{
				//ジャスト回避成功
				hits.push_back({ character, true });
				continue;
			}
		}

		//2つのカプセルの半径を合計した値が実際の当たり判定の距離になる
		float hitRange = attackdata.GetRadius() + capsule.radius;

		if (distance <= hitRange)
		{
			hits.push_back({ character, false });
		}
	}
	return hits;
}

//当たり判定を描画する(デバッグ用)
void CollisionManager::DrawDebug()const
{
	for (auto character : m_pCharacters)
	{
		//当たり判定(黄色)
		Geometry::Capsule capsule = character->GetCollisionCapsule();
		DrawCapsule3D(capsule.start, capsule.end, capsule.radius, 8, GetColor(255, 255, 0), GetColor(255, 255, 0), false);
		//ジャスト回避判定(水色)
		Geometry::Capsule justDodge = character->GetJustDodgeCapsule();
		if (justDodge.radius > 0.0f)
		{
			DrawCapsule3D(justDodge.start, justDodge.end, justDodge.radius, 8, GetColor(0, 255, 255), GetColor(0, 255, 255), false);
		}
	}

	//このフレームに判定した攻撃(緑)
	for (auto& shape : m_debugAttackShapes)
	{
		DrawCapsule3D(shape.start, shape.end, shape.radius, 8, GetColor(0, 255, 0), GetColor(0, 255, 0), false);
	}
}

//重なっている2つのキャラクターを、それぞれ反対方向へ押し出して重なりを解消する
void CollisionManager::ResolveCharacterCollision(Character* characterA, Character* characterB)
{
	Geometry::Capsule a = characterA->GetCollisionCapsule();
	Geometry::Capsule b = characterB->GetCollisionCapsule();

	//2本の軸の最近接点を求める
	Vector3 closestA, closestB;
	Geometry::ClosestPointSegmentSegment(a.start, a.end, b.start, b.end, closestA, closestB);

	Vector3 direction = closestA - closestB;

	//Y方向は無視してXZ平面のみで押し出す
	direction.y = 0.0f;

	float distanceSq = direction.SqMagnitude();

	//ほぼ同じ位置にいる場合は方向を求められないため、デフォルトの方向を使用する
	if (distanceSq < 0.000001f)
	{
		//真上/真下に重なっている場合の保険として、任意方向にずらす
		direction = Vector3(1.0f, 0.0f, 0.0f);
		distanceSq = 1.0f;
	}
	//押し出し方向を正規化
	float distance = sqrtf(distanceSq);
	direction /= distance;
	//２つのカプセルの半径を合計
	float combinedRadius = a.radius + b.radius;
	//カプセル同士が重なっている量
	float penetration = combinedRadius - distance;
	//重なっていなければ何もしない
	if (penetration <= 0.0f)
	{
		return;
	}
	//2人を均等に押し出し
	float push = penetration * 0.5f;

	Vector3 posA = characterA->GetPos();
	Vector3 posB = characterB->GetPos();

	posA += direction * push;
	posB -= direction * push;

	characterA->SetPosition(posA);
	characterB->SetPosition(posB);
}

//2つのキャラクターのカプセルが重なっているか判定する
bool CollisionManager::CheckCharacterCapsule(Character* characterA, Character* characterB)
{
	//ここで純粋な幾何計算関数に投げる
	return Geometry::CheckCapsule(characterA->GetCollisionCapsule(), characterB->GetCollisionCapsule());
}
