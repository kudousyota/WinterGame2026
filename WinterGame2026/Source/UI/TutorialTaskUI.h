#pragma once
#include "UIBase.h"

class TutorialGuide;
struct TutorialStep;
class TutorialTaskUI : public UIBase
{
public:
	//TutorialSceneが持っていてUIより長く残るので借りるだけにする
	explicit TutorialTaskUI(const TutorialGuide& guide);

	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

	//ステップを飛ばす長押しの進み具合//0は押していない、1で飛ばす
	void SetSkipRate(float rate) { m_skipRate = rate; }

private:
	//右端につけた帯を描く
	void DrawBand(int left, int centerY, int height)const;
	//真ん中から右上へどれだけ動いたか//0は真ん中で止まっている、1は右上に着いた
	float GetMoveRate()const;
	//ウィッチタイム中に真ん中へ一言出す//敵が遅い間に攻撃すればいいと気づかせる
	void DrawWitchTimeMessage()const;

	//課題を読む進行役//所有しない//読むだけなのでconst
	const TutorialGuide* m_pGuide = nullptr;
	//最後に見たステップ//変わったら真ん中から出し直す
	const TutorialStep* m_shownStep = nullptr;
	//今のステップを出してから経ったフレーム数
	int m_frame = 0;
	//ステップを飛ばす長押しの進み具合//TutorialSceneが毎フレーム入れる
	float m_skipRate = 0.0f;
	//ウィッチタイムに入ってから経ったフレーム数//入っていないときは-1
	int m_witchTimeFrame = -1;
};
