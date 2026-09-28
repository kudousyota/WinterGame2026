#include "Camera.h"
#include "DxLib.h"
//#include "../System/CollisionManager.h"
//#include "../Game/Stage.h"
//#include "../System/Model.h"
//#include "../System/Timer.h" 

namespace
{
	//注視点からカメラに向かうベクトル
	const Vector3 kTargetToCamera = Vector3(0.0f, 200.0f, 800.0f);

	//注視点からカメラまでの距離
	constexpr float kCameraDistance = 300.0f;
	//カメラの高さ
	constexpr float kCameraHeight = -250.0f;

	//補間
	constexpr float kCameraFollow = 0.15f;

	constexpr float kStickDeadZone = 0.001f;
	constexpr float kCameraRotSpeedX = 1.2f;
	constexpr float kCameraRotSpeedY = 1.0f;
	//ジャスト回避の時にひくカメラの距離
	constexpr float kJustAvoidPullDistance = 150.0f;
	//引いたカメラが元に戻る速さ
	constexpr float kPullRecoverySpeed = 0.3f;
}

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Init(const Vector3& initialTarget, float initialAngle)
{

	m_cameraAngleX = initialAngle;
	m_cameraAngleY = 0.0f;
	m_cameraTargetY = initialTarget.y;

	Vector3 offset;
	offset.x = sinf(m_cameraAngleX) * cosf(m_cameraAngleY) * kCameraDistance;
	offset.y = sinf(m_cameraAngleY) * kCameraDistance;
	offset.z = cosf(m_cameraAngleX) * cosf(m_cameraAngleY) * kCameraDistance;

	m_cameraTarget = initialTarget;
	m_cameraPos = initialTarget - offset;

	SetCameraPositionAndTarget_UpVecY(m_cameraPos.ToDxLibVector(), m_cameraTarget.ToDxLibVector());

}

void Camera::End()
{
}
//targetpos追従する基準点
void Camera::Update(const Vector3& targetPos)
{

		int xBuf = 0, yBuf = 0;

		//右スティック入力取得
		GetJoypadAnalogInputRight(&xBuf, &yBuf, DX_INPUT_PAD1);
#ifdef _DEBUG


		DrawFormatString(256, 128, 0xffffff, "Right Stick X : %d", xBuf);
		DrawFormatString(256, 144, 0xffffff, "Right Stick Y : %d", yBuf);
		DrawFormatString(256, 160, 0xffffff, "CameraAngle : %.3f", m_cameraAngleX);
#endif //_DEBUG

		//正規化
		float stickX = static_cast<float>(xBuf) / 32767.0f;
		float stickY = static_cast<float>(yBuf) / 32767.0f;

		//デッドゾーン
		if (fabsf(stickX) < kStickDeadZone) stickX = 0.0f;
		if (fabsf(stickY) < kStickDeadZone) stickY = 0.0f;

		//カメラ左右回転
		if (stickX != 0.0f)
		{
			m_cameraAngleX += stickX * kCameraRotSpeedX;
		}
		if (stickY != 0.0f)
		{
			m_cameraAngleY -= stickY * kCameraRotSpeedY;
		}

	const float kMaxPitch = DX_PI_F * 0.4f;
	const float kMinPitch = -DX_PI_F * 0.2f;
	//上下移動の制限
	if (m_cameraAngleY > kMaxPitch)
	{
		m_cameraAngleY = kMaxPitch;
	}

	if (m_cameraAngleY < kMinPitch)
	{
		m_cameraAngleY = kMinPitch;
	}

	//角度を-π～πに収める
	if (m_cameraAngleX > DX_PI_F)  m_cameraAngleX -= DX_TWO_PI_F;
	if (m_cameraAngleX < -DX_PI_F) m_cameraAngleX += DX_TWO_PI_F;

	////ウィッチタイム中はカメラを引く
	//if (Timer::Instance().IsEnemySlow())
	//{
	//	//目標値に近づける
	//	m_pullOffset += (kJustAvoidPullDistance - m_pullOffset) * kPullRecoverySpeed;
	//}
	//else
	//{
	//	//ウィッチタイムが終わったら徐々に戻す
	//	m_pullOffset += (0.0f - m_pullOffset) * kPullRecoverySpeed;
	//	if (fabsf(m_pullOffset) < 0.01f)
	//	{
	//		m_pullOffset = 0.0f;
	//	}
	//}

	//プレイヤー追従先
	Vector3 cameraTarget = targetPos;

	//引いたことを加味したカメラの距離
	float currentDistace = kCameraDistance + m_pullOffset;

	//カメラの位置
	Vector3 offset;
	offset.x = sinf(m_cameraAngleX) * cosf(m_cameraAngleY) * currentDistace;
	offset.y = sinf(m_cameraAngleY) * currentDistace;
	offset.z = cosf(m_cameraAngleX) * cosf(m_cameraAngleY) * currentDistace;

	//プレイヤーの後ろから見るようにするなら「-offset」
	Vector3 idealCameraPos = cameraTarget - offset;

	Vector3 hitpos;

	////ステージとのレイ判定を行い、カメラがステージにめり込まないようにする
	//if (CollisionManager::Instance().CheckCameraRay(
	//	m_pStage->GetModelHandle(),
	//	m_cameraTarget,
	//	m_cameraPos,
	//	hitpos))
	//{
	//	Vector3 dir = (m_cameraTarget - hitpos).Normalize();
	//	m_cameraPos = hitpos + dir * 10.0f;
	//}

	//追従
	m_cameraPos = m_cameraPos + (idealCameraPos - m_cameraPos) * kCameraFollow;
	m_cameraTarget = m_cameraTarget + (cameraTarget - m_cameraTarget) * kCameraFollow;

	////カメラシェイク
	//Vector3 shakeOffset = Timer::Instance().GetShakeOffset();
	//Vector3 shakenCameraPos = m_cameraPos + shakeOffset;
	//Vector3 shakenCameraTarget = m_cameraTarget + shakeOffset;

	//カメラ反映
	//SetCameraPositionAndTarget_UpVecY(shakenCameraPos.ToDxLibVector(), shakenCameraTarget.ToDxLibVector());

	//カメラ反映
	SetCameraPositionAndTarget_UpVecY(m_cameraPos.ToDxLibVector(), m_cameraTarget.ToDxLibVector());

	//光
	Vector3 camPos = m_cameraPos;
	Vector3 target = m_cameraTarget;

	////光
	//Vector3 camPos = shakenCameraPos.ToDxLibVector();
	//Vector3 target = shakenCameraTarget.ToDxLibVector();

	//カメラから注視点の方向
	Vector3 LightDir = VNorm(VSub(target, camPos));

	SetLightDirection(LightDir);

}

void Camera::Draw()
{

}

Vector3 Camera::GetForward() const
{
	Vector3 forward = m_cameraTarget - m_cameraPos;
	forward.y = 0.0f;

	if (forward.SqMagnitude() > 0.0001f)
	{
		forward = forward.Normalize();
	}
	return forward;
}

Vector3 Camera::GetRight() const
{
	Vector3 forward = GetForward();
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	Vector3 right = forward.Cross(up);

	if (right.SqMagnitude() > 0.0001f)
	{
		right = right.Normalize();
	}
	//間違っていたから反転
	right *= -1;

	return right;
}
//
//void Camera::CameraPull()
//{
//	//ジャスト回避にカメラを引く
//	m_pullOffset = kJustAvoidPullDistance;
//}
