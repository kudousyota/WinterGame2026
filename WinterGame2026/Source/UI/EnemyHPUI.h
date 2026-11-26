#pragma once
#include <memory>
#include <vector>
#include "UIBase.h"
#include "HPGauge.h"

class GameManager;
class Character;
class EnemyHPUI : public UIBase
{
public:
	//game:敵の一覧とロックオンしている敵を見る//GameSceneがUIより長く持っているので借りるだけにする
	explicit EnemyHPUI(const GameManager& game);
	~EnemyHPUI();
	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

private:
	//敵1体分のHPバー
	struct Bar
	{
		//対象の敵//所有しない//敵が消えるとlockが空を返す
		std::weak_ptr<const Character> target;
		//HPのゲージ
		HPGauge gauge;
		//前のフレームのHP//これより減っていたらダメージを受けたと判断する
		int prevHp = 0;
		//表示する残りフレーム
		int showFrame = 0;
		//透明度//0で見えない、255で不透明
		int alpha = 0;
		//頭の上の画面上の位置
		float screenX = 0.0f;
		float screenY = 0.0f;
		//画面に映っているか//カメラの後ろにいると映らない
		bool isOnScreen = false;
	};

	//敵の一覧と照らし合わせて新しく出てきた敵のバーを足し消えた敵のバーを外す
	void SyncBars();
	//1体分のバーの表示時間、透明度、画面上の位置を決める
	void UpdateBar(Bar& bar, const Character& enemy, bool isLockOnTarget);

	//敵の一覧とロックオンしている敵を見る//所有しない
	const GameManager* m_pGame = nullptr;
	//敵ごとのHPバー
	std::vector<Bar> m_bars;
	//黄色のゲージの画像ハンドル//全員のHPGaugeに貸す
	int m_gaugeHandle = -1;
	//減った分を見せる赤いゲージの画像ハンドル//全員のHPGaugeに貸す
	int m_trailHandle = -1;
	//ゲージの後ろの枠の画像ハンドル
	int m_frameHandle = -1;
};
