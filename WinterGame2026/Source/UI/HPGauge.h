#pragma once

//HPの残りをゲージで描く部品
//プレイヤーのHPUIと敵のHPUIで共通に使う
//画像は使う側が読み込んで渡す//ここでは読み込みも削除もしない
//敵の数だけvectorで持つとコピーされるので、ここで削除すると画像が消えてしまう
class HPGauge
{
public:
	//current:今のHP max:最大HP
	//減った分を遅れて追いかける赤いゲージもここで進める
	void Update(int current, int max);
	//x,y:ゲージの左上 width,height:満タンのときの大きさ
	//gaugeHandle:幅に合わせて引き延ばす画像//借りるだけ
	//alpha:透明度//0で見えない、255で不透明//フェードアウトに使う
	void Draw(float x, float y, float width, float height, int gaugeHandle, int alpha = 255) const;
	//減った分を赤で描く//Drawより先に呼んで、本当のゲージの後ろに描く
	//ゲージの画像に縁があるときは縁の内側に合わせられるよう、場所は別に受け取る
	//trailHandle:LoadTrailGraphで作った赤いゲージの画像//借りるだけ
	void DrawTrail(float x, float y, float width, float height, int trailHandle, int alpha = 255) const;

	//赤いゲージの画像を読み込む//使う側が持って、いらなくなったらDeleteGraphする
	//元の画像は大きな透明の中に細い帯があるだけなので、帯だけを切り出した画像にする
	static int LoadTrailGraph();

	//今の残りの割合//0から1
	float GetRate() const { return m_rate; }

private:
	//今の残りの割合//0から1
	float m_rate = 1.0f;
	//遅れて追いかける赤いゲージの割合//0から1
	//0から始めると最初のUpdateで今の割合にそろう
	float m_trailRate = 0.0f;
	//最後に減ってからのしばらくは赤いゲージを止めて、どれだけ削れたか見せる
	int m_trailWaitFrame = 0;
};
