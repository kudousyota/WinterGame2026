#pragma once
#include "../../../Math/Vector3.h"
#include <vector>

class PlayerCape
{
public:
	//なびき方の種類//Playerがステートに合わせて選ぶ
	enum class Mode
	{
		Idle,//待機//そよ風でゆったり揺れる
		Move,//移動//向かい風で大きく後ろへ流れて細かくはためく
		Air,//空中//上がる時は垂れ、落ちる時は大きくめくれ上がる
		Num
	};

	void Init(int modelHandle, const Vector3& pos);
	//なびき方を切り替える//急に変わらないよう少しずつ切り替わる
	void SetMode(Mode mode) { m_mode = mode; }
	//モデルの位置、向き、アニメーションを反映した後に呼ぶ
	void Update(const Vector3& pos, const Vector3& forward, float timeScale);

private:
	//なびき方ごとの風の強さ
	struct WindParam
	{
		//風でいつも後ろへ傾く角度
		float baseAngle;
		//風の強弱で前後に揺れる角度
		float gustAngle;
		//風の向きが変わって左右に揺れる角度
		float sideAngle;
		//はためきの大きさ
		float waveAngle;
		//はためきの速さ
		float waveSpeed;
	};

	//風で決まる目標の傾き
	Vector3 CalcWindTilt(const Vector3& forward)const;
	//今の傾きを骨に反映する
	void ApplyToBones(const Vector3& forward);

private:
	int m_modelHandle = -1;
	//マントの骨//根元から先の順に並べる
	std::vector<int> m_frames;
	//骨の元の姿勢
	std::vector<MATRIX> m_baseMatrices;

	Mode m_mode = Mode::Idle;
	//今の風//なびき方の値へ少しずつ近づける
	WindParam m_wind = {};

	//前のフレームの位置//動いた量を出すのに使う
	Vector3 m_prevPos;
	//マント全体の傾き
	//向きがワールドでマントの先が流れる方向、長さが傾く角度
	Vector3 m_tilt;
	//傾きの変わる速さ//バネの計算に使う
	Vector3 m_tiltVelocity;
	//風の強弱を出すための時間
	float m_windTime = 0.0f;
	//はためきの波の位相
	float m_wavePhase = 0.0f;
};
