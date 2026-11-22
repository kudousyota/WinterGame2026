#pragma once
#include "Vector3.h"
class Character;

namespace StageCollision
{
	//キャラクターとステージの当たり判定をまとめて解決する
	//壁から押し出した後、地面に立たせる
	void Resolve(Character& character, int stagehandle);
	//キャラクターとステージの壁との衝突を判定しめり込んでいた場合は壁の外側へ押し出す
	bool CheckWall(Character& character, int stagehandle);
	//ステージの地面
	bool CheckGround(Character& character, int stagehandle, float& outGroundY, Vector3& outGroundNormal);
	//真下にある地面の高さを探す
	bool FindGroundY(int stagehandle, float x, float z, float baseY, float& outGroundY);
	//カメラとステージの判定
	bool CheckCameraRay(int stagehandle, const Vector3& start, const Vector3& end, Vector3& hitpos);
	//前の位置から今の位置までレイを飛ばし壁を超えていたら壁の手前に戻す
	bool SweepWall(Character& character, int stagehandle);
}
