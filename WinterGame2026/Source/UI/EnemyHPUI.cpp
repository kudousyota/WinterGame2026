#include "EnemyHPUI.h"
#include <algorithm>
#include "DxLib.h"
#include "../Game/GameManager.h"
#include "../Game/Character/Enemy/Enemy.h"

namespace
{
	//ダメージを受けてから表示し続けるフレーム数
	constexpr int kShowFrame = 180;
	//消えるときにフェードアウトするフレーム数//表示の残りがこれより少なくなったら薄くしていく
	constexpr int kFadeFrame = 30;

	//画像を描く倍率//これを変えると枠とゲージが一緒に大きさを変える
	constexpr float kScale = 0.1f;
	//頭のてっぺんから枠の下の端までの画面上の距離
	//画面上で離す敵の大きさや距離に関係なく同じ見た目になる
	constexpr float kHeadMargin = 8.0f;

	//ここから下は枠の画像の中のピクセル位置
	//枠の画像は1280x720で、まわりが透明になっている

	//枠の画像の中心//DrawRotaGraphはここを基準に描く
	constexpr float kFrameCenterX = 640.0f;
	constexpr float kFrameCenterY = 360.0f;
	//枠の中のゲージを入れる場所の中心//バーの基準点にする
	constexpr float kInnerCenterX = 681.0f;
	constexpr float kInnerCenterY = 324.0f;
	//ゲージを入れる場所の左端と幅
	constexpr float kInnerLeft = 263.0f;
	constexpr float kInnerWidth = 836.0f;
	//ゲージの画像の上端と高さ
	//ゲージの画像は上下に黒い縁があるので、枠の黒い縁にぴったり重なる位置にする
	constexpr float kGaugeTop = 279.0f;
	constexpr float kGaugeHeight = 90.0f;
	//枠の飾りの下の端
	constexpr float kFrameBottom = 380.0f;

	//画像は回さない
	constexpr float kAngle = 0.0f;
	//不透明
	constexpr int kMaxAlpha = 255;
}

EnemyHPUI::EnemyHPUI(const GameManager& game)
{
	m_pGame = &game;
}

EnemyHPUI::~EnemyHPUI()
{
	DeleteGraph(m_gaugeHandle);
	DeleteGraph(m_frameHandle);
}

void EnemyHPUI::Init()
{
	//画像のロード
	m_gaugeHandle = LoadGraph("Data/Image/EnemyHP.png");
	m_frameHandle = LoadGraph("Data/Image/EnemyHPBer.png");
}

void EnemyHPUI::Update(Input& input)
{
	SyncBars();

	//ロックオンしている敵はフレームの間は変わらないので先に1回だけ取る
	const std::shared_ptr<const Character> lockOnTarget = m_pGame->GetLockOnTarget();
	for (Bar& bar : m_bars)
	{
		const std::shared_ptr<const Character> enemy = bar.target.lock();
		//SyncBarsで外しているから空にはならないが念のため
		if (!enemy) continue;
		UpdateBar(bar, *enemy, enemy == lockOnTarget);
	}
}

void EnemyHPUI::Draw()
{
	for (const Bar& bar : m_bars)
	{
		if (!bar.isOnScreen) continue;
		if (bar.alpha <= 0) continue;

		//基準点はゲージを入れる場所の中心
		//枠の飾りの下の端が頭のてっぺんからkHeadMarginだけ上に来るようにする
		const float baseX = bar.screenX;
		const float baseY = bar.screenY - kHeadMargin - (kFrameBottom - kInnerCenterY) * kScale;

		//枠//中のゲージを入れる場所が半透明の暗い色なのでゲージの背景にもなる
		//大きく縮めるのでギザギザにならないようにバイリニアで描く
		SetDrawMode(DX_DRAWMODE_BILINEAR);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, bar.alpha);
		DrawRotaGraphF(baseX + (kFrameCenterX - kInnerCenterX) * kScale, baseY + (kFrameCenterY - kInnerCenterY) * kScale, kScale, kAngle, m_frameHandle, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		SetDrawMode(DX_DRAWMODE_NEAREST);

		//ゲージ//枠の中のゲージを入れる場所に合わせる
		const float gaugeLeft = baseX + (kInnerLeft - kInnerCenterX) * kScale;
		const float gaugeTop = baseY + (kGaugeTop - kInnerCenterY) * kScale;
		bar.gauge.Draw(gaugeLeft, gaugeTop, kInnerWidth * kScale, kGaugeHeight * kScale, m_gaugeHandle, bar.alpha);
	}
}

void EnemyHPUI::SyncBars()
{
	//消えた敵のバーを外す
	std::erase_if(m_bars, [](const Bar& bar) { return bar.target.expired(); });

	//新しく出てきた敵のバーを足す
	for (const std::shared_ptr<Character>& enemy : m_pGame->GetEnemies())
	{
		const bool hasBar = std::any_of(m_bars.begin(), m_bars.end(),
			[&enemy](const Bar& bar) { return bar.target.lock() == enemy; });
		if (hasBar) continue;
		//ボスは画面の下のBossHPUIが出すので頭の上には出さない
		const Enemy* asEnemy = dynamic_cast<const Enemy*>(enemy.get());
		if (asEnemy && asEnemy->IsBoss()) continue;

		Bar bar;
		bar.target = enemy;
		//出てきたときのHPを覚えておく//最初のフレームでダメージを受けたと間違えないように
		bar.prevHp = enemy->GetHp();
		bar.gauge.Update(enemy->GetHp(), enemy->GetMaxHp());
		m_bars.push_back(bar);
	}
}

void EnemyHPUI::UpdateBar(Bar& bar, const Character& enemy, bool isLockOnTarget)
{
	//死んだ敵のバーはすぐ消す
	if (enemy.IsDead())
	{
		bar.showFrame = 0;
		bar.alpha = 0;
		return;
	}

	//HPが減っていたらダメージを受けたので表示し直す
	const int hp = enemy.GetHp();
	if (hp < bar.prevHp)
	{
		bar.showFrame = kShowFrame;
	}
	bar.prevHp = hp;
	bar.gauge.Update(hp, enemy.GetMaxHp());

	if (isLockOnTarget)
	{
		//ロックオン中は減らさずにずっと表示する
		//外したときにいきなり消えずにフェードアウトするよう残りをフェードの分だけは残しておく
		bar.showFrame = (std::max)(bar.showFrame, kFadeFrame);
		bar.alpha = kMaxAlpha;
	}
	else
	{
		if (bar.showFrame > 0) bar.showFrame--;
		//残りがフェードのフレーム数より少なくなったら薄くしていく
		bar.alpha = kMaxAlpha * (std::min)(bar.showFrame, kFadeFrame) / kFadeFrame;
	}

	//見えないバーは位置を計算しない
	if (bar.alpha <= 0) return;

	//頭のてっぺんを画面上の位置に変える
	//カメラはGameManagerのUpdateで確定しているのでその後に呼ばれるここで計算できる
	const VECTOR screenPos = ConvWorldPosToScreenPos(enemy.GetHeadPos().ToDxLibVector());
	//zが0から1の外ならカメラの後ろか描ける範囲の外
	bar.isOnScreen = (screenPos.z > 0.0f && screenPos.z < 1.0f);
	bar.screenX = screenPos.x;
	bar.screenY = screenPos.y;
}
