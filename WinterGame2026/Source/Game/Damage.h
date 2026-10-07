#pragma once
class AttackData;
class Character;

//ダメージ計算
//最終ダメージはすべてここを通して求める
//ウィッチタイム中の倍率や難易度補正などは、ここに足すだけで済むようにする
namespace Damage
{
	int Calc(const AttackData& attack, const Character& attacker, const Character& target);
}
