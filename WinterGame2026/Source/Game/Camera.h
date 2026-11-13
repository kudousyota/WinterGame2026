#pragma once
#include <optional>
#include "../Math/Vector3.h"

//プレイヤーの後ろから追いかけるカメラ
class Camera
{
public:
	//射影の設定と最初の向き位置を決める
	void Init(const Vector3& focusPos, const Vector3& forward);

	//lockOnHeight:ロックオンしている敵の身長(大きい敵ほど下から見上げる)
	void Update(const Vector3& focusPos, const Vector3& PlayerForward,const std::optional<Vector3>& lockOnPos, float lockOnHeight);

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
	void UpdateLockOnAngle(const Vector3& focusPos, const Vector3& lockOnPos, float lockOnHeight);
	//カメラリセット中ならプレイヤーの背中側へ少しずつ回す
	void UpdateRecenter(const Vector3& playerForward);
	//縦の制限と横を-π～πに収める
	void ClampAngle();
	//引きの量を補間する
	void UpdateDistance();
	//注視点から見たカメラの向き
	Vector3 CalcDir() const;
	//注視点から見てposを画面の内側に入れるのに必要なカメラの距離
	float CalcFitDistance(const Vector3& pos, const Vector3& dir) const;
	//ステージに当たらずに離れられる距離(当たらなければidealDistのまま)
	float CalcStageDistance(const Vector3& dir, float idealDist) const;
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
	//右スティックを触っていないフレーム(戦闘の構図を自動で調整するときに使う)
	int m_noInputFrame = 0;
	//カメラリセットで背中側へ戻している途中か
	bool m_isRecentering = false;

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
	//カメラシェイクの全体のフレーム数
	int m_shakeTotalFrame = 0;
	//壁を考えない、本来の注視点からカメラの距離
	float m_baseDistance = 0.0f;
	//今の注視点からカメラの距離
	float m_curDistance = 0.0f;

};
