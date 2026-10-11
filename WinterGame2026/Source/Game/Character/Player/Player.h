#pragma once
#include "../Character.h"
class Player : public Character
{
public:
	Player();
	~Player()override;

	void Init()override;
	void Update()override;
	void LateUpdate()override;
	void Draw()override;

	CharacterType GetCharacterType()const override { return CharacterType::Player; }

	//ジャスト回避に成功したらウィッチタイムを発動する
	void OnJustDodge()override;

	Vector3 GetCameraTarget();
	float GetAngle() { return m_angle; }

	//カメラ基準で移動するためにカメラの向きを設定する
	void SetCameraAxis(const Vector3& forward, const Vector3& right);
	
	const Vector3& GetCameraForward()const { return m_cameraForward; }
	const Vector3& GetCameraRight()const { return m_cameraRight; }
	
	//すぐ振り向く
	void FaceTo(const Vector3& dir);

	void SetAnimPause(bool isPause) { m_isAnimPause = isPause; }

private:
	int m_modelHandle = -1;
	bool m_isAnimPause = false;
	//カメラの前方向(XZ平面)
	Vector3 m_cameraForward = Vector3(0.0f, 0.0f, 1.0f);
	//カメラの右方向(XZ平面)
	Vector3 m_cameraRight = Vector3(1.0f, 0.0f, 0.0f);
};

