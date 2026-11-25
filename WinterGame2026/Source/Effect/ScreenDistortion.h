#pragma once

class ScreenDistortion
{
public:
	//シェーダーの定数バッファと並びを合わせる
	struct Param
	{
		float centerX = 0.5f;	//波の中心
		float centerY = 0.5f;
		float progress = 0.0f;	//0→1で波が広がる
		float strength = 0.0f;	//ずらす強さ//UV単位で、0なら歪まない
		float aspect = 1.0f;	//画面の幅、高さ
		float ringWidth = 0.08f;//輪の太さ
		float time = 0.0f;		//揺らぎを動かす時間
		float waveStrength = 0.0f;//揺らぎの強さ
		float gradeAmount = 0.0f;//ウィッチタイムの色にする割合//0で元の色、1で暗い藍色と紫
		float gradeRadius = 100.0f;//この半径より内側だけ色を変える//大きくしておけば画面全体
		float vignette = 0.0f;	//画面の四隅を暗くする強さ
		float padding = 0.0f;	//シェーダーの定数バッファは16バイト単位なのでそろえる
	};

	ScreenDistortion() = default;
	~ScreenDistortion();
	//コピーするとハンドルを二重に消すので禁止
	ScreenDistortion(const ScreenDistortion&) = delete;
	ScreenDistortion& operator=(const ScreenDistortion&) = delete;

	void Init();
	void Terminate();

	//歪みの設定(Updateで決めた値を渡す)
	void SetParam(const Param& param) { m_param = param; }
	const Param& GetParam() const { return m_param; }

	//ここから描いたものを歪ませる
	void BeginCapture();
	//ここまでに描いたもの(ステージ)だけをウィッチタイムの色にする
	//この後に描くプレイヤーと敵には色がつかない//奥行きの情報はそのまま残る
	void ApplyGrade();
	//描き終わったら歪ませて画面に出す
	void EndCapture();

private:
	//シェーダーを通してtextureを今の描画先の全画面に描く
	void DrawWithShader(int pixelShader, int texture);

	Param m_param;
	//ゲーム画面をいったん描いておくスクリーン
	int m_screen = -1;
	//ステージを描いた時点の画面を写しておくスクリーン//同じ画面を読みながら書けないため
	int m_copyScreen = -1;
	//歪みのシェーダー
	int m_pixelShader = -1;
	//ウィッチタイムの色のシェーダー
	int m_gradeShader = -1;
	int m_constantBuffer = -1;
};
