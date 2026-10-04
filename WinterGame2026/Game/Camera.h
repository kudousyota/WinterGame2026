#pragma once
#include <memory>
#include "../Math/Vector3.h"
class Stage;
class Camera
{
public:
	Camera();
	~Camera();
	void Init(const Vector3& initialTarget, float initialAngle);
	void End();
	void Update(const Vector3& targetPos);
	void Draw();
	//void SetStage(std::shared_ptr<Stage> stage) { m_pStage = stage; }
	Vector3 GetForward() const;
	Vector3 GetRight() const;

	//ウィッチタイムになったらカメラをひく
	//void CameraPull();

	const Vector3& GetPos()const { return m_cameraPos; }
	//カメラの水平・垂直の回転角度
	float GetAngleX()const { return m_cameraAngleX; }
	float GetAngleY()const { return m_cameraAngleY; }


private:
	//プレイヤーへの参照を保持
	std::shared_ptr<Stage> m_pStage;

	//カメラの位置
	Vector3 m_cameraTarget = Vector3(0.0f,0.0f,0.0f);
	Vector3 m_cameraPos  = Vector3(0.0f, 0.0f, 0.0f);

	//カメラの角度
	//カメラの水平回転角度
	float m_cameraAngleX = 0.0f;
	//カメラの垂直回転角度
	float m_cameraAngleY = 0.0f;

	float m_cameraTargetY = 0.0f;
	//ジャスト回避した時に引く量
	float m_pullOffset = 0.0f;
};