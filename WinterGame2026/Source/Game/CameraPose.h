#pragma once
#include "../Math/Vector3.h"

//カメラの位置と見る点
//普段のカメラと演出用のカメラの間で受け渡す共通の形
struct CameraPose
{
	//カメラの位置
	Vector3 pos;
	//見る点
	Vector3 target;
};
