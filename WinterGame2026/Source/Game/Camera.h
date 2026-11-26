#pragma once
#include <optional>
#include "../Math/Vector3.h"
#include "CameraPose.h"

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

	//今のポーズ(揺れは入っていない)
	CameraPose GetPose() const { return CameraPose{ m_pos, m_focus }; }
	//ポーズに揺れを足してDxLibに反映しライトの向きを合わせる
	//演出用のカメラのポーズもここを通すので、演出中も揺れる
	void Apply(const CameraPose& pose);
	//演出用のカメラから普段のカメラへ戻すときに呼ぶ
	//そのポーズから角度・距離・注視点を作り直すので、戻した瞬間に画面が飛ばない
	//縦の角度は制限(kMinPitch～kMaxPitch)の中に収まるので、外れるショットの後は少し動く
	void SyncFromPose(const CameraPose& pose);

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
	//注視点を追いかける
	void UpdateFocus(const Vector3& focusGoal);
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
	//演出用のカメラから戻した後、注視点を目標まで戻している途中か
	//この間は縦の遅れの上限を使わない(上限で一気に詰めるとカメラが飛ぶため)
	bool m_isFocusReturning = false;

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
