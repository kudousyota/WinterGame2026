#include "StageCollision.h"
#include "../Game/Character/Character.h"
#include <cfloat>
#include <cmath>

namespace
{
	//歩行可能な最大傾斜角度
	constexpr float kWalkableSlopeAngleDeg = 60.0f;
	//歩行可能な傾斜角度を判定するためのcos値
	//法線のY成分と比較して、床・坂道と壁を判別する
	const float kWalkableSlopeCos = cosf(kWalkableSlopeAngleDeg * (DX_PI_F / 180.0f));
}

//キャラクターとステージの当たり判定をまとめて解決する
void StageCollision::Resolve(Character* character, int stagehandle)
{
	//先に壁から押し出して、XZの位置を確定させる
	CheckWall(character, stagehandle);

	//確定した位置の真下にある地面を探す
	float groundY = 0.0f;
	Vector3 groundNormal;
	bool isHit = CheckGround(character, stagehandle, groundY, groundNormal);

	Vector3 pos = character->GetPos();
	Vector3 velocity = character->GetVelocity();

	//上昇中は着地しない(ジャンプした直後に地面へ引き戻されないようにする)
	bool isRising = velocity.y > 0.0f;
	//地面にいたキャラクターは、レイが届く範囲なら地面に吸着する(坂や段差を下りるときに浮かないようにする)
	//空中にいたキャラクターは、足元が地面に届いたときだけ着地する(落下中に地面へ瞬間移動しないようにする)
	bool canLand = character->IsGround() || pos.y <= groundY;

	if (isHit && !isRising && canLand)
	{
		pos.y = groundY;
		velocity.y = 0.0f;
		character->SetPosition(pos);
		character->SetVelocity(velocity);
		character->SetGround(true);
	}
	else
	{
		character->SetGround(false);
	}
}

//キャラクターとステージの壁との衝突を判定し、めり込んでいた場合は壁の外側へ押し出す
bool StageCollision::CheckWall(Character* character, int stagehandle)
{
	//プレイヤーのカプセル情報を取得
	float radius = character->GetCollisionRadius();
	float height = character->GetCollisionHeight();

	//足元基準の現在地
	Vector3 pos = character->GetPos();
	//このフレームで一回でもポリゴンに当たったか確認
	bool hitAny = false;

	//当たっていたら
	//4回判定する
	const int kMaxIteration = 4;
	for (int iter = 0; iter < kMaxIteration; iter++)
	{
		//カプセルの始点と終点を作成
		//(キャラ同士・攻撃判定と同じカプセル形状にそろえる)
		VECTOR start = Vector3(pos.x, pos.y + radius, pos.z);
		VECTOR end = VAdd(start, Vector3(0.0f, height, 0.0f));
		//ステージとのカプセル衝突判定
		auto hit = MV1CollCheck_Capsule(stagehandle, -1, start, end, radius);
		//何もヒットしなければ,これ以上押し出す必要はない
		if (hit.HitNum == 0)
		{
			MV1CollResultPolyDimTerminate(hit);
			break;
		}
		//ポリゴンに当たったか記録
		//ここでは壁ポリゴンに当たったかを一旦保留にする
		//一番めり込んだポリゴンを探すための変数
		float maxDepth = -FLT_MAX;
		//一番深いポリゴンの法線を保持
		Vector3 bestNormal;
		//当たったポリゴンをチェック
		for (int i = 0; i < hit.HitNum; i++)
		{
			Vector3 fullNormal(hit.Dim[i].Normal.x, hit.Dim[i].Normal.y, hit.Dim[i].Normal.z);
			fullNormal = fullNormal.Normalize();

			//歩ける床と坂とみなして壁判定から除外
			//歩行可能な坂道は壁として扱わない
			if (fullNormal.y >= kWalkableSlopeCos)
			{
				//坂と床はスキップ
				continue;
			}
			//法線のY成分を無視してXZ平面に投射
			Vector3 rawNormal(hit.Dim[i].Normal.x, 0.0f, hit.Dim[i].Normal.z);

			//ほぼ水平なポリゴン床は壁判定から除外
			float lenSq = rawNormal.SqMagnitude();
			if (lenSq < 0.0001f)
			{
				continue;
			}
			//歩行可能な坂道は壁として扱わない
			Vector3 normal(hit.Dim[i].Normal.x, 0.0f, hit.Dim[i].Normal.z);

			normal = normal.Normalize();
			//ポリゴンの頂点の一つを平面上の点として使う
			Vector3 planepoint(hit.Dim[i].Position[0].x,
				hit.Dim[i].Position[0].y,
				hit.Dim[i].Position[0].z);

			//カプセル軸
			Vector3 axispoint(pos.x, planepoint.y, pos.z);
			//平面点からカプセル軸への距離を法線方向に射影
			float signedDist = normal.Dot(axispoint - planepoint);

			//半径よりも平面に近ければめり込んでいる
			float depth = radius - signedDist;
			//最も深くめり込んでいるポリゴンを記録
			if (depth > maxDepth)
			{
				maxDepth = depth;
				bestNormal = normal;
			}

		}
		//実際に壁として押し出しが発生した場合のみ hitAny を立てる
		//実際にめり込んでいた場合のみ、その分だけ壁の法線方向に押し出す
		if (maxDepth > 0.0f)
		{
			pos += bestNormal * maxDepth;
			hitAny = true;
		}
		//ヒット結果のメモリを解放
		MV1CollResultPolyDimTerminate(hit);
	}
	//押し出し後の最終的な座標をキャラクターに反映
	character->SetPosition(pos);
	//壁に一度でも当たったかを返す
	return hitAny;
}

bool StageCollision::CheckGround(Character* character, int stagehandle, float& outGroundY, Vector3& outGroundNormal)
{
	//キャラクターの現在の位置を取得
	Vector3 pos = character->GetPos();
	//キャラクターのカプセルの半径を取得
	float radius = character->GetCollisionRadius();
	//レイを伸ばして坂道や段差でも設置判定をできるようにする
	const float kUpMargin = 5.0f;
	const float kDownMargin = 8.0f;

	//レイ(下向きの線分)の開始点と終了点を決める
	//キャラクターの基準位置は足元
	//カプセル半径を考慮した位置から下向きにレイを飛ばす
	VECTOR rayStart = Vector3(pos.x, pos.y + radius + kUpMargin, pos.z);
	VECTOR rayEnd = Vector3(pos.x, pos.y - radius - kDownMargin, pos.z);

	//ステージとのレイ判定をする
	MV1_COLL_RESULT_POLY hitPoly = MV1CollCheck_Line(stagehandle, -1, rayStart, rayEnd);
	//レイが地面に当たったら
	if (hitPoly.HitFlag == 1)
	{
		//当たった場所を返す
		outGroundY = hitPoly.HitPosition.y;
		//地面の法線を正規化して返す
		outGroundNormal = Vector3(hitPoly.Normal.x, hitPoly.Normal.y, hitPoly.Normal.z).Normalize();
		return true;
	}
	//地面が見つからなかったら
	return false;

}
//始点から終点へレイを飛ばし、ステージに遮られた場合は衝突位置を返す
bool StageCollision::CheckCameraRay(int stagehandle, const Vector3& start, const Vector3& end, Vector3& hitpos)
{
	MV1_COLL_RESULT_POLY result = MV1CollCheck_Line(stagehandle, -1, start, end);
	//ステージにヒットした場合
	if (result.HitFlag)
	{
		hitpos.x = result.HitPosition.x;
		hitpos.y = result.HitPosition.y;
		hitpos.z = result.HitPosition.z;
		return true;
	}
	return false;
}
