#pragma once
class AttackData;
class Character;
class Vector3;

//ダメージ計算
//最終ダメージはすべてここを通して求める
//ウィッチタイム中の倍率や難易度補正などは、ここに足すだけで済むようにする
namespace Damage
{
	
	//startからendは当たった攻撃の線分//弱点に当たったかを調べる
	int Calc(const AttackData& attack, const Character& attacker, const Character& target,const Vector3& start,const Vector3& end);
}
