#pragma once

//HPの残りをゲージで描く部品
//プレイヤーのHPUIと敵のHPUIで共通に使う
//画像は使う側が読み込んで渡す//ここでは読み込みも削除もしない
//敵の数だけvectorで持つとコピーされるので、ここで削除すると画像が消えてしまう
class HPGauge
{
public:
	//current:今のHP max:最大HP
	void Update(int current, int max);
	//x,y:ゲージの左上 width,height:満タンのときの大きさ
	//gaugeHandle:幅に合わせて引き延ばす画像//借りるだけ
	//alpha:透明度//0で見えない、255で不透明//フェードアウトに使う
	void Draw(float x, float y, float width, float height, int gaugeHandle, int alpha = 255) const;

	//今の残りの割合//0から1
	float GetRate() const { return m_rate; }

private:
	//今の残りの割合//0から1
	float m_rate = 1.0f;
};
