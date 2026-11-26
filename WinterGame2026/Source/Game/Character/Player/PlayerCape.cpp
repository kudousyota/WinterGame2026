#include "PlayerCape.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
	//マントの骨の数//根元(背中の上)がCloak1、裾がCloak7
	constexpr int kBoneNum = 7;
	//骨ごとに全体の傾きを受け持つ割合(合計1)//裾ほど大きく曲げると布らしく丸くなる
	//根元は肩とつながっているので小さくして、肩から離れないようにする
	constexpr float kBoneWeights[kBoneNum] = { 0.04f, 0.10f, 0.13f, 0.16f, 0.18f, 0.19f, 0.20f };
	//骨ごとのはためきの大きさの割合//根元は動かさず裾ほど大きく波打つ
	constexpr float kWaveWeights[kBoneNum] = { 0.0f, 0.15f, 0.25f, 0.35f, 0.45f, 0.55f, 0.65f };

	//なびき方ごとの風の強さ(ラジアン)//Modeの順に並べる
	//baseAngle, gustAngle, sideAngle, waveAngle, waveSpeed
	constexpr float kWindParams[][5] =
	{
		{ 0.45f, 0.30f, 0.25f, 0.35f, 0.10f },//Idle//そよ風でゆったり大きく揺れる
		{ 0.30f, 0.15f, 0.15f, 0.45f, 0.40f },//Move//向かい風で後ろへ流れ、細かくはためく
		{ 0.55f, 0.20f, 0.30f, 0.80f, 0.50f },//Air//風を受けてばたばたはためく
	};
	static_assert(sizeof(kWindParams) / sizeof(kWindParams[0]) == static_cast<int>(PlayerCape::Mode::Num),
		"kWindParams must have one row per Mode");
	//なびき方を切り替える時に1フレームで近づく割合
	constexpr float kModeBlendRate = 0.06f;

	//風の強弱と向きの揺れの速さ//違う速さの波を重ねて、同じ揺れの繰り返しに見えないようにする
	constexpr float kGustSpeeds[3] = { 0.031f, 0.077f, 0.013f };
	constexpr float kGustRates[3] = { 0.6f, 0.3f, 0.1f };
	constexpr float kSideSpeeds[2] = { 0.023f, 0.059f };
	constexpr float kSideRates[2] = { 0.7f, 0.3f };

	//横に動く速さ1あたりに後ろへ傾く角度(ラジアン)//歩く速さ12で約20度
	constexpr float kSpeedToAngle = 0.03f;
	//落ちる速さ1あたりにめくれ上がる角度(ラジアン)
	constexpr float kFallToAngle = 0.07f;
	//上がる速さ1あたりに傾きが減る割合//上がる時はマントが下へ引っ張られて垂れる
	constexpr float kRiseToDroop = 0.06f;
	//上がる時でも残す傾きの割合
	constexpr float kMinRiseRate = 0.25f;
	//傾きの上限(ラジアン)//約95度で、裾が真横より少し上までめくれる
	constexpr float kMaxAngle = 1.65f;
	//体の前へ入り込んでいい角度(ラジアン)//止まった時の揺り戻しで少しだけ前に出る
	constexpr float kMaxFrontAngle = 0.15f;

	//バネの強さ//大きいほど目標の傾きにすぐ追いつく
	constexpr float kStiffness = 0.07f;
	//1フレームで残る揺れの割合//小さいほど揺れがすぐ収まる
	constexpr float kDamping = 0.85f;

	//この速さで動くと移動によるはためきが一番大きくなる
	constexpr float kFullSpeed = 12.0f;
	//止まっている時のはためきの割合
	constexpr float kWaveRateMin = 0.5f;
	//骨1本ごとの波のずれ//根元から裾へ波が流れて見える
	constexpr float kWavePhaseStep = 0.8f;

	//1フレームでこれ以上動いたら瞬間移動とみなしてマントを振らない
	constexpr float kTeleportSpeed = 200.0f;

	//これより小さい角度は曲げない//向きが決まらず回転が不安定になるのを防ぐ
	constexpr float kMinAngle = 0.0001f;
}

void PlayerCape::Init(int modelHandle, const Vector3& pos)
{
	m_modelHandle = modelHandle;
	m_frames.clear();
	m_baseMatrices.clear();
	for (int i = 0; i < kBoneNum; i++)
	{
		std::string name = "mixamorig:Cloak" + std::to_string(i + 1);
		int frame = MV1SearchFrame(m_modelHandle, name.c_str());
		//1本でも見つからなければ動かさない//骨がずれて形が崩れるのを防ぐ
		if (frame < 0)
		{
			m_frames.clear();
			m_baseMatrices.clear();
			return;
		}
		m_frames.push_back(frame);
		m_baseMatrices.push_back(MV1GetFrameBaseLocalMatrix(m_modelHandle, frame));
	}

	//やり直したときに前の揺れが残らないようにする
	m_mode = Mode::Idle;
	const float* param = kWindParams[static_cast<int>(m_mode)];
	m_wind = { param[0], param[1], param[2], param[3], param[4] };
	m_prevPos = pos;
	m_tilt = Vector3(0.0f, 0.0f, 0.0f);
	m_tiltVelocity = Vector3(0.0f, 0.0f, 0.0f);
	m_windTime = 0.0f;
	m_wavePhase = 0.0f;
}

void PlayerCape::Update(const Vector3& pos, const Vector3& forward, float timeScale)
{
	if (m_frames.empty()) return;

	//止まっている間(ヒットストップなど)は前の形のまま
	if (timeScale <= 0.0f)
	{
		m_prevPos = pos;
		return;
	}

	//今のなびき方の風へ少しずつ近づける//切り替えた瞬間にマントが跳ねないようにする
	const float* param = kWindParams[static_cast<int>(m_mode)];
	float blend = (std::min)(kModeBlendRate * timeScale, 1.0f);
	m_wind.baseAngle += (param[0] - m_wind.baseAngle) * blend;
	m_wind.gustAngle += (param[1] - m_wind.gustAngle) * blend;
	m_wind.sideAngle += (param[2] - m_wind.sideAngle) * blend;
	m_wind.waveAngle += (param[3] - m_wind.waveAngle) * blend;
	m_wind.waveSpeed += (param[4] - m_wind.waveSpeed) * blend;
	m_windTime += timeScale;

	//このフレームの速さ
	Vector3 velocity = (pos - m_prevPos) / timeScale;
	m_prevPos = pos;
	//やり直しや出現で位置が飛んだ時は動いていないことにする
	if (velocity.SqMagnitude() > kTeleportSpeed * kTeleportSpeed)
	{
		velocity = Vector3(0.0f, 0.0f, 0.0f);
	}

	//目標の傾きを決める
	//風を受けて後ろへ流れる
	Vector3 target = CalcWindTilt(forward);
	//動いた方向の逆へも流れる
	Vector3 moveXZ(velocity.x, 0.0f, velocity.z);
	target -= moveXZ * kSpeedToAngle;
	//後ろ歩きでも体の前へは流れないようにする
	float front = target.Dot(forward);
	if (front > 0.0f)
	{
		target -= forward * front;
	}
	if (velocity.y > 0.0f)
	{
		//上がっている時は下へ引っ張られて垂れる
		target *= (std::max)(1.0f - velocity.y * kRiseToDroop, kMinRiseRate);
	}
	else
	{
		//落ちている時は下からの風でめくれ上がる
		target -= forward * (-velocity.y * kFallToAngle);
	}
	float targetAngle = target.Magnitude();
	if (targetAngle > kMaxAngle)
	{
		target *= kMaxAngle / targetAngle;
	}

	//バネで目標の傾きを追いかける//少し遅れて揺り戻すので布らしくなる
	m_tiltVelocity += (target - m_tilt) * (kStiffness * timeScale);
	m_tiltVelocity *= powf(kDamping, timeScale);
	m_tilt += m_tiltVelocity * timeScale;

	//揺り戻しで体にめり込まないよう、前に出る量を抑える
	float tiltFront = m_tilt.Dot(forward);
	if (tiltFront > kMaxFrontAngle)
	{
		m_tilt -= forward * (tiltFront - kMaxFrontAngle);
		//前へ向かう勢いも消して、押し付け続けないようにする
		float velFront = m_tiltVelocity.Dot(forward);
		if (velFront > 0.0f)
		{
			m_tiltVelocity -= forward * velFront;
		}
	}

	//速く動いているほど速くはためく
	float speedRate = (std::min)(moveXZ.Magnitude() / kFullSpeed, 1.0f);
	m_wavePhase += m_wind.waveSpeed * (kWaveRateMin + (1.0f - kWaveRateMin) * speedRate) * timeScale;

	ApplyToBones(forward);
}

Vector3 PlayerCape::CalcWindTilt(const Vector3& forward) const
{
	//風の強弱(-1～1)//違う速さの波を重ねる
	float gust = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		gust += sinf(m_windTime * kGustSpeeds[i]) * kGustRates[i];
	}
	//風の向きの揺れ(-1～1)
	float side = 0.0f;
	for (int i = 0; i < 2; i++)
	{
		side += sinf(m_windTime * kSideSpeeds[i] + i * 1.3f) * kSideRates[i];
	}

	//風は体の前から吹いてくることにして、マントを後ろへ流す
	Vector3 back = forward * -1.0f;
	Vector3 right(forward.z, 0.0f, -forward.x);
	return back * (m_wind.baseAngle + m_wind.gustAngle * gust) + right * (m_wind.sideAngle * side);
}

void PlayerCape::ApplyToBones(const Vector3& forward)
{
	const Vector3 down(0.0f, -1.0f, 0.0f);
	//はためく方向//傾いている方向、ほとんど傾いていなければ背中側
	Vector3 waveDir = forward * -1.0f;
	if (m_tilt.SqMagnitude() > kMinAngle * kMinAngle)
	{
		waveDir = m_tilt.Normalize();
	}

	for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
	{
		const int frame = m_frames[i];

		//この骨が受け持つ傾き(ワールド)
		//全体の傾きを分けた分に、骨ごとに波をずらしたはためきを足す
		float wave = sinf(m_wavePhase - i * kWavePhaseStep) * m_wind.waveAngle * kWaveWeights[i];
		Vector3 tilt = m_tilt * kBoneWeights[i] + waveDir * wave;
		float angle = tilt.Magnitude();

		MATRIX rot = MGetIdent();
		if (angle > kMinAngle)
		{
			//真下を、傾ける方向へangleだけ倒した向きにする回転
			Vector3 to = down * cosf(angle) + (tilt / angle) * sinf(angle);
			//回転は親の骨の空間で掛けるので、ワールドの向きを親の空間へ直す
			//親の向きには前の骨で曲げた分も入っているので、根元から順に曲がりが積み重なる
			MATRIX parentInv = MInverse(MV1GetFrameLocalWorldMatrix(m_modelHandle, MV1GetFrameParent(m_modelHandle, frame)));
			VECTOR fromLocal = VNorm(VTransformSR(down, parentInv));
			VECTOR toLocal = VNorm(VTransformSR(to, parentInv));
			rot = MGetRotVec2(fromLocal, toLocal);
		}

		//元の姿勢の回転の後に曲げる回転を掛け、関節の位置はそのまま
		MATRIX base = m_baseMatrices[i];
		VECTOR trans = VGet(base.m[3][0], base.m[3][1], base.m[3][2]);
		base.m[3][0] = 0.0f;
		base.m[3][1] = 0.0f;
		base.m[3][2] = 0.0f;
		MATRIX mat = MMult(MMult(base, rot), MGetTranslate(trans));
		MV1SetFrameUserLocalMatrix(m_modelHandle, frame, mat);
	}
}
