#include "WitchTimeNeedleUI.h"
#include "../System/Timer.h"
#include "../Game.h"
#include "DxLib.h"

WitchTimeNeedleUI::WitchTimeNeedleUI()
{
}

WitchTimeNeedleUI::~WitchTimeNeedleUI()
{
	m_needleHandle = DeleteGraph(m_needleHandle);
}

void WitchTimeNeedleUI::Init()
{
	m_needleHandle = LoadGraph("Data/Image/ui_niidle.png");
}

void WitchTimeNeedleUI::Update(Input& input)
{
	//敵がスロー中ならウィッチタイム
	bool currentWitch = Timer::Instance().IsEnemySlow();
	//ウィッチタイムに入ったときに演出をする
	//今の状態&&前の状態
	if (currentWitch && !m_isPrevWitchTime)
	{
		//表示する時間
		m_witchEffectFrame = 80;
		//回転角度
		m_angle = 0.0f;
	}

	//次のフレームで判定するために保存
	m_isPrevWitchTime = currentWitch;
	//演出中だったら回転させる
	if (m_witchEffectFrame > 0)
	{
		//演出フレームをここで減らす
		m_witchEffectFrame--;

		//回転角度を増やす
		m_angle += DX_PI_F / 40.0f;

		//0から360度まで回転させる
		//360度以上にならないように
		if (m_angle > DX_TWO_PI_F)
		{
			m_angle -= DX_TWO_PI_F;
		}

	}
}

void WitchTimeNeedleUI::Draw()
{
	//ウィッチタイムでなければ描画しない
	if (!Timer::Instance().IsEnemySlow()) return;
	//演出が終わっていたら描画しない
	if (m_witchEffectFrame <= 0) return;

	//画面の真ん中に回転する画像を描画する
	DrawRectRotaGraph2(Game::kScreenWidth / 2, Game::kScreenHeight / 2, 0, 0, 256, 512, 128, 256, 1.5f, m_angle, m_needleHandle, true, false);
}
