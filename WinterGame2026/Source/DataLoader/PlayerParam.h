#pragma once
#include <vector>
#include <string>

struct PlayerParam
{
	//CSVの行データを受け取る
	//CSVに無い名前があったら例外を投げる//ReloadDataの読み直しが失敗扱いになり今の値が残る
	void Load(const std::vector<std::vector<std::string>>& rows);

	//移動
	//止まった状態から最高速になるまでのフレーム数
	int accelFrame = 0;
	//最高速から止まるまでのフレーム数
	int decelFrame = 0;
	//1フレームで目標の向きへ回る割合
	float rotateRate = 0.0f;

	//ジャンプ
	//ジャンプした瞬間の上向きの速さ
	float jumpPower = 0.0f;
	//1フレームごとに加える下向きの速さ
	float gravity = 0.0f;
	//空中で動ける速さの倍率
	float airMoveSpeedScale = 0.0f;

	//回避
	//回避にかかるフレーム数
	int dodgeFrame = 0;
	//回避中の移動速度の倍率
	float dodgeSpeedScale = 0.0f;
	//ジャスト回避を受け付けるフレーム数
	int justDodgeFrame = 0;
	//ジャスト回避判定の半径
	float justDodgeRadius = 0.0f;

	//ウィッチタイム
	//続くフレーム数
	int witchTimeFrame = 0;
	//敵の時間の倍率
	float witchTimeScale = 0.0f;

	//被弾
	//被弾後の無敵フレーム数
	int hitInvincibleFrame = 0;

	//入力
	//先行入力を覚えておくフレーム数
	int inputBufferFrame = 0;

	//攻撃
	//攻撃判定を出す位置
	float hitForwardOffset = 0.0f;
	float hitHeight = 0.0f;
	//敵が遠いとき前進距離をCSVの値の何倍まで伸ばすか
	float maxMoveScale = 0.0f;
	//敵の手前で止まるときに体同士の間に空ける隙間
	float stopMargin = 0.0f;
	//ウィケッドウィーブの攻撃判定を出す位置
	float witchHandForwardOffset = 0.0f;

	//長押しで撃つ弾
	//弾を撃つ間隔のフレーム数
	int shotInterval = 0;
	float bulletSpeed = 0.0f;
	int bulletDamage = 0;
	float bulletRadius = 0.0f;
};
