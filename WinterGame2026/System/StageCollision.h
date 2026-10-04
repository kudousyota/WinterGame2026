#pragma once
#include "Vector3.h"
class Character;

//ステージモデルとの当たり判定
//キャラクターの登録リストを使わないのでCollisionManagerとは分けている
namespace StageCollision
{
	//キャラクターとステージの壁との衝突を判定し、めり込んでいた場合は壁の外側へ押し出す
	bool CheckWall(Character* character, int stagehandle);
	//ステージの地面
	bool CheckGround(Character* character, int stagehandle, float& outGroundY, Vector3& outGroundNormal);
	//カメラとステージの判定
	bool CheckCameraRay(int stagehandle, const Vector3& start, const Vector3& end, Vector3& hitpos);
}
