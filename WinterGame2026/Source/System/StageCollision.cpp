#include "StageCollision.h"
#include "../Game/Character/Character.h"
#include <cfloat>
#include <cmath>

namespace
{
	//歩行可能な最大傾斜角度
	constexpr float kWalkableSlopeAngleDeg = 60.0f;
	//歩行可能な傾斜角度を判定するためのcos値
	//法線のY成分と比較して、床や坂道と壁を判別する
	const float kWalkableSlopeCos = cosf(kWalkableSlopeAngleDeg * (DX_PI_F / 180.0f));
	//登れる段差の高さ
	constexpr float kStepHeight = 30.0f;
}

//キャラクターとステージの当たり判定をまとめて解決する
void StageCollision::Resolve(Character& character, int stagehandle)
{
	//先に壁から押し出して、XZの位置を確定させる
	CheckWall(character, stagehandle);

	//確定した位置の真下にある地面を探す
	float groundY = 0.0f;
	Vector3 groundNormal;
	bool isHit = CheckGround(character, stagehandle, groundY, groundNormal);

	Vector3 pos = character.GetPos();
	Vector3 velocity = character.GetVelocity();

	//上昇中は着地しない(ジャンプした直後に地面へ引き戻されないようにする)
	bool isRising = velocity.y > 0.0f;
	//地面にいたキャラクターは、レイが届く範囲なら地面に吸着する(坂や段差を下りるときに浮かないようにする)
	//空中にいたキャラクターは、足元が地面に届いたときだけ着地する(落下中に地面へ瞬間移動しないようにする)
	bool canLand = character.IsGround() || pos.y <= groundY;

	if (isHit && !isRising && canLand)
	{
		pos.y = groundY;
		velocity.y = 0.0f;
		character.SetPosition(pos);
		character.SetVelocity(velocity);
		character.SetGround(true);
	}
	else
	{
		character.SetGround(false);
	}
}

//キャラクターとステージの壁との衝突を判定しめり込んでいた場合は壁の外側へ押し出す
bool StageCollision::CheckWall(Character& character, int stagehandle)
{
	//早く動いて壁を超えていたら先に壁の前に戻す
	bool hitAny = SweepWall(character, stagehandle);

	//プレイヤーのカプセル情報を取得
	float radius = character.GetCollisionRadius();
	float height = character.GetCollisionHeight();

	//足元基準の現在地
	Vector3 pos = character.GetPos();
	//当たっていたら
	//4回判定する
	const int kMaxIteration = 4;
	for (int iter = 0; iter < kMaxIteration; iter++)
	{
		//カプセルの始点と終点を作成
		//(キャラ同士や攻撃判定と同じカプセル形状にそろえる)
		VECTOR start = Vector3(pos.x, pos.y +kStepHeight + radius, pos.z);
		VECTOR end = VAdd(start, Vector3(0.0f, height, 0.0f));
		//ステージとのカプセル衝突判定
		auto hit = MV1CollCheck_Capsule(stagehandle, -1, start, end, radius);
		//何もヒットしなければ,これ以上押し出す必要はない
		if (hit.HitNum == 0)
		{
			MV1CollResultPolyDimTerminate(hit);
			break;
		}
		//一番深くめり込んだ深さ//0のままならどのポリゴンにもめり込んでいない
		float maxDepth = 0.0f;
		//一番深いポリゴンから押し出す向き
		Vector3 bestPush;
		//当たったポリゴンをチェック
		for (int i = 0; i < hit.HitNum; i++)
		{
			Vector3 fullNormal(hit.Dim[i].Normal.x, hit.Dim[i].Normal.y, hit.Dim[i].Normal.z);
			fullNormal = fullNormal.Normalize();

			//歩ける床と坂は壁として扱わない
			if (fullNormal.y >= kWalkableSlopeCos) continue;

			//カプセルの軸と三角形のいちばん近い点どうしを求める
			//面を無限に広げて測ると、角や壁と壁の間でとなりの面に押し込まれて挟まる
			SEGMENT_TRIANGLE_RESULT result;
			Segment_Triangle_Analyse(&start, &end,
				&hit.Dim[i].Position[0], &hit.Dim[i].Position[1], &hit.Dim[i].Position[2], &result);

			//半径より離れていればめり込んでいない
			float depth = radius - sqrtf(result.Seg_Tri_MinDist_Square);
			if (depth <= maxDepth) continue;

			//三角形から軸へ向かう向きに押し出す//高さは地面の判定で決めるのでXZだけ
			Vector3 push(result.Seg_MinDist_Pos.x - result.Tri_MinDist_Pos.x, 0.0f,
				result.Seg_MinDist_Pos.z - result.Tri_MinDist_Pos.z);
			if (push.SqMagnitude() < 0.0001f)
			{
				//軸がポリゴンを貫いていて向きが出ないときは、面の向きで押し出す
				push = Vector3(fullNormal.x, 0.0f, fullNormal.z);
				if (push.SqMagnitude() < 0.0001f) continue;
			}
			maxDepth = depth;
			bestPush = push.Normalize();
		}
		//めり込んでいたときだけ、その分だけ押し出す
		if (maxDepth > 0.0f)
		{
			pos += bestPush * maxDepth;
			hitAny = true;
		}
		//ヒット結果のメモリを解放
		MV1CollResultPolyDimTerminate(hit);
	}
	//押し出し後の最終的な座標をキャラクターに反映
	character.SetPosition(pos);
	//壁に一度でも当たったかを返す
	return hitAny;
}

bool StageCollision::CheckGround(Character& character, int stagehandle, float& outGroundY, Vector3& outGroundNormal)
{
	//キャラクターの現在の位置を取得
	Vector3 pos = character.GetPos();
	//レイを伸ばして坂道や段差でも設置判定をできるようにする
	const float kUpMargin = 5.0f;
	const float kDownMargin = 8.0f;

	//レイの開始点と終了点を決める
	//キャラクターの基準位置は足元
	//登れる段差の高さより少し上から飛ばす//半径を足すと体の大きいキャラほど高い壁に乗り上げてしまう
	VECTOR rayStart = Vector3(pos.x, pos.y + kStepHeight + kUpMargin, pos.z);
	//下りの坂や段差で浮かないように足元より少し下まで届かせる
	VECTOR rayEnd = Vector3(pos.x, pos.y - kStepHeight - kDownMargin, pos.z);

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
bool StageCollision::FindGroundY(int stagehandle, float x, float z, float baseY, float& outGroundY)
{
	//基準の高さより上の段差も拾えるよう、少し上から調べる
	constexpr float kSearchUp = 200.0f;
	constexpr float kSearchDown = 1000.0f;
	VECTOR rayStart = VGet(x, baseY + kSearchUp, z);
	VECTOR rayEnd = VGet(x, baseY - kSearchDown, z);

	MV1_COLL_RESULT_POLY hitPoly = MV1CollCheck_Line(stagehandle, -1, rayStart, rayEnd);
	if (hitPoly.HitFlag == 0) return false;
	outGroundY = hitPoly.HitPosition.y;
	return true;
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

bool StageCollision::SweepWall(Character& character, int stagehandle)
{
	float radius = character.GetCollisionRadius();
	Vector3 pos = character.GetPos();
	const Vector3& prevPos = character.GetPrevPos();
	
	//レイの始点と終点//終点も始点と同じ高さにする
	//登れる段差のすぐ上を通す//半径を足すと体の大きいキャラほど低い壁の上を素通りしてしまう
	const float lineY = prevPos.y + kStepHeight + 1.0f;
	Vector3 start(prevPos.x ,lineY, prevPos.z);
	Vector3 end(pos.x, lineY, pos.z);

	//動いていなかったら調べない
	Vector3 move = end - start;
	if (move.SqMagnitude() < 0.00001f)return false;

	//線を飛ばす
	MV1_COLL_RESULT_POLY hitPoly = MV1CollCheck_Line(stagehandle, -1, start, end);
	if (hitPoly.HitFlag == 0)return false;

	//超えては行けない壁を調べる
	Vector3 normal = Vector3(hitPoly.Normal.x, hitPoly.Normal.y, hitPoly.Normal.z).Normalize();
	//床とか坂ではない
	if (normal.y >= kWalkableSlopeCos) return false;

	//移動方向と壁の向きはxとzだけで比べる
	Vector3 dir = move.Normalize();
	Vector3 wallNormal = Vector3(normal.x, 0.0f, normal.z);

	//壁の裏から当たったとき//押し出されて離れている途中は止めない
	if (dir.Dot(wallNormal) >= 0.0f) return false;

	//当たった場所から来た方向分もどす
	Vector3 hitPos (hitPoly.HitPosition.x, hitPoly.HitPosition.y, hitPoly.HitPosition.z);
	pos.x = hitPos.x - dir.x * radius;
	pos.z = hitPos.z - dir.z * radius;
	character.SetPosition(pos);
	return true;
}
