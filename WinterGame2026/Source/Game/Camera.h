#pragma once
#include <optional>
#include "../Math/Vector3.h"

//プレイヤーの後ろから追いかけるカメラ
class Camera
{
public:
	//射影の設定と、最初の向き・位置を決める
	void Init(const Vector3& focusPos, float yaw);
	//focusPos:追いかける位置 lockOnPos:ロックオン中の敵の位置
	void Update(const Vector3& focusPos, const std::optional<Vector3>& lockOnPos);

	//めり込み判定に使うステージのモデル
	void SetStageHandle(int handle) { m_stageHandle = handle; }
	//カメラを引くかどうか
	void SetPull(bool isPull) { m_isPull = isPull; }
	//カメラを揺らす
	void Shake(float power, int frame);

	//水平の正面と右
	Vector3 GetForward() const;
	Vector3 GetRight() const;

	const Vector3& GetPos() const { return m_pos; }
	//横回転の角度
	float GetYaw() const { return m_yaw; }
	//縦回転の角度
	float GetPitch() const { return m_pitch; }

private:
	//右スティックで回す
	void UpdateFreeAngle();
	//ロックオン中の角度
	void UpdateLockOnAngle(const Vector3& focusPos, const Vector3& lockOnPos);
	//縦の制限と横を-π～πに収める
	void ClampAngle();
	//引きの量を補間する
	void UpdateDistance();
	//注視点から見たカメラの向き
	Vector3 CalcDir() const;
	//ステージにめり込まない位置にする
	Vector3 AvoidStage(const Vector3& idealPos) const;
	//揺れを足してDxLibに反映しライトの向きを合わせる
	void Apply();
	//今フレームの揺れの量を求める
	Vector3 CalcShakeOffset() const;

private:
	//横回転の角度
	float m_yaw = 0.0f;
	//縦回転の角度
	float m_pitch = 0.0f;
	//引いている量
	float m_pullOffset = 0.0f;
	bool m_isPull = false;

	//補間した注視点
	Vector3 m_focus = Vector3(0.0f, 0.0f, 0.0f);
	//カメラの位置
	Vector3 m_pos = Vector3(0.0f, 0.0f, 0.0f);

	//ステージのモデル
	int m_stageHandle = -1;

	//カメラシェイクの強さ
	float m_shakePower = 0.0f;
	//カメラシェイクの残りフレーム数
	int m_shakeFrame = 0;
	//カメラシェイクの全体のフレーム数(だんだん弱くするのに使う)
	int m_shakeTotalFrame = 0;
	//今の注視点からカメラの距離
	float m_curDistance = 0.0f;

};
