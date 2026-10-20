#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include "AttackData.h"

//攻撃ごとのパラメータ(フレームやダメージ)をCSVから読み込んで持っておくクラス
class AttackTable
{
public:
	struct AttackInfo
	{
		//攻撃の種類
		AttackType type = AttackType::Punch;
		//攻撃全体のフレーム数
		int totalFrame = 0;
		//攻撃判定が出るフレーム
		int hitStartFrame = 0;
		//攻撃判定が消えるフレーム
		int hitEndFrame = 0;
		//次の攻撃の入力を受け付け始めるフレーム
		int comboStartFrame = 0;
		//次の攻撃が予約されていたら、このフレームで次の攻撃に切り替える
		int cancelFrame = 0;
		//ダメージ
		int damage = 0;
		//攻撃判定の半径
		float radius = 0.0f;
		//前進を始めるフレーム
		int moveStartFrame = 0;
		//前進を終えるフレーム
		int moveEndFrame = 0;
		//進む距離
		float moveDistance = 0.0f;
		//アニメーションの名前
		std::string animName;
		//コンボ中にパンチを押したとき次に出す攻撃(なければ空)
		std::optional<AttackType> nextPunch;
		//コンボ中にキックを押したとき次に出す攻撃(なければ空)
		std::optional<AttackType> nextKick;
		//コンボの締めで攻撃判定が出たときに呼び出すウィケッドウィーブ(なければ空)
		std::optional<AttackType> witchHand;
	};

	//CSVの行データを受け取る
	void Load(const std::vector<std::vector<std::string>>& rows);

	//攻撃の種類からパラメータを取得
	const AttackInfo& GetInfo(AttackType type)const;
private:
	std::unordered_map<AttackType, AttackInfo> m_infoList;
};
