#pragma once
#include "../Character.h"
class WickedWeave;
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

	Vector3 GetMoveInput()const;

	//攻撃する時に向く相手を指定する
	void SetAttackTarget(const std::shared_ptr<const Character>& target) { m_attackTarget = target; }

	void SetGravityScale(float scale) { m_gravityScale = scale; }

	//向く相手
	std::shared_ptr<const Character> GetAttackTarget()const { return m_attackTarget.lock(); }
	//空中で浮ける権利が残っているか
	bool CanAirHover()const { return m_canAirHover; }
	//空中で浮ける権利を使う
	void UseAirHover();

	//二段ジャンプの権利があるか
	bool CanDobuleJump()const { return m_canDoubleJump; }
	//二段ジャンプの権利を使う
	void UseDoubuleJump();

	//ウィケッドウィーブを出す
	//出した後は状態が変わっても最後まで攻撃を続ける
	void AddWickedWeave(std::unique_ptr<WickedWeave> weave);

	//攻撃を受けたらのけぞる
	HitResult OnHit(const AttackData& attackData, int damage)override;

	//読み直したステータスとPlayerParam.csvの値を反映する
	void ReloadStatus()override;
private:
	//PlayerParam.csvの値のうちメンバーに持つものを入れる
	void ApplyParam();
	//出ているウィケッドウィーブの攻撃判定を出して終わったものを消す
	void UpdateWickedWeaves();

	int m_modelHandle = -1;
	bool m_isAnimPause = false;
	//カメラの前方向
	Vector3 m_cameraForward = Vector3(0.0f, 0.0f, 1.0f);
	//カメラの右方向
	Vector3 m_cameraRight = Vector3(1.0f, 0.0f, 0.0f);
	float m_gravityScale = 1.0f;
	bool m_canAirHover = false;
	bool m_canDoubleJump = false;
	//所有せずシーンから向く相手を指定してもらう
	//敵が消えても触らないようにweak_ptrで持つ
	std::weak_ptr<const Character> m_attackTarget;
	//出ているウィケッドウィーブ//複数同時に出せる
	std::vector<std::unique_ptr<WickedWeave>> m_wickedWeaves;
};

