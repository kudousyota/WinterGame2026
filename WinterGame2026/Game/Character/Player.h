#pragma once
#include "Character.h"
class Player : public Character
{
public:
	Player();
	~Player()override;

	void Init()override;
	void Update()override;
	void Draw()override;

	//キャラクタータイプを取得
	CharacterType GetCharacterType()const override { return CharacterType::Player; }

	Vector3 GetCameraTarget();
	float GetAngle() { return m_angle; }

	//カメラ基準で移動するためにカメラの向きを設定する
	void SetCameraAxis(const Vector3& forward, const Vector3& right);
	
	const Vector3& GetCameraForward()const { return m_cameraForward; }
	const Vector3& GetCameraRight()const { return m_cameraRight; }
private:
	int m_modelHandle = -1;

	//カメラの前方向(XZ平面)
	Vector3 m_cameraForward = Vector3(0.0f, 0.0f, 1.0f);
	//カメラの右方向(XZ平面)
	Vector3 m_cameraRight = Vector3(1.0f, 0.0f, 0.0f);
};

