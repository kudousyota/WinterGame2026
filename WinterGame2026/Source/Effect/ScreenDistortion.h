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
		float strength = 0.0f;	//ずらす強さ//UV単位0なら歪まない
		float aspect = 1.0f;	//画面の幅、高さ
		float ringWidth = 0.08f;//輪の太さ
		float time = 0.0f;		//揺らぎを動かす時間
		float waveStrength = 0.0f;//揺らぎの強さ
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
	//描き終わったら歪ませて画面に出す
	void EndCapture();

private:
	//シェーダーを通して全画面に描く
	void DrawWithShader();

	Param m_param;
	//ゲーム画面をいったん描いておくスクリーン
	int m_screen = -1;
	int m_pixelShader = -1;
	int m_constantBuffer = -1;
};
