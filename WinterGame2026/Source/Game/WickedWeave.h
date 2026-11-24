#pragma once
#include "Vector3.h"
#include "../DataLoader/AttackTable.h"
#include "../Effect/EffectManager.h"
#include "Character/Character.h"

class WickedWeave
{
public:
	//呼び出すときに決めること
	struct SummonParam
	{
		//攻撃判定の中心//ワールド座標
		Vector3 hitPos;
		//見た目のエフェクト
		EffectType effectType = EffectType::WickedWeave;
		Vector3 effectPos;
		Vector3 effectRot;
		float effectScale = 1.0f;
	};

	/// <summary>
	/// 呼び出した瞬間にエフェクトを出す
	/// </summary>
	/// <param name="info">攻撃のデータ//判定の時間やダメージに使う</param>
	/// <param name="attackId">足技とは別の番号にして、足技が当たった相手にも当たるようにする</param>
	/// <param name="param">位置とエフェクト</param>
	WickedWeave(const AttackTable::AttackInfo& info, int attackId, const SummonParam& param);

	//1フレーム進める
	void Update();
	//今攻撃判定が出ているか
	bool IsHitActive()const;
	//このフレームの攻撃の申請を作る//持ち主がRequestAttackに渡す
	AttackRequest MakeAttackRequest()const;
	//攻撃判定の時間が終わったか//終わったら持ち主がリストから外す
	bool IsDead()const;

private:
	//CSVを読み直しても途中で中身が変わらないようにコピーで持つ
	AttackTable::AttackInfo m_info;
	//攻撃番号
	int m_attackId = 0;
	//攻撃判定の中心
	Vector3 m_hitPos;
	//呼び出してからのフレーム数
	int m_frame = 0;
	//見た目のエフェクト
	int m_effectHandle = -1;
};
