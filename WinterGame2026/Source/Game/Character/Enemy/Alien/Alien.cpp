#include "Alien.h"
#include "../../../../Common/Model.h"
#include "../State/EnemyStateIdle.h"
#include <cmath>

namespace
{
	const char* const kIdleAnimName = "Alien|Idle";
	const char* const kMoveAnimName = "Alien|Move";

	//最初の位置(プレイヤーと重ならないように少し奥に置く)
	const Vector3 kStartPos = Vector3(0.0f, 0.0f, 300.0f);

	//体の大きさ(デバッグ表示のカプセルを見ながら調整する)
	//身長
	constexpr float kHeight = 200.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 50.0f;

	//索敵範囲(この距離までプレイヤーを追いかける)
	constexpr float kSearchRange = 1000.0f;
	//攻撃範囲(この距離まで近づいたら攻撃する)
	constexpr float kAttackRange = 150.0f;

	//回転の補完
	constexpr float kRotateRate = 0.15f;
}

Alien::Alien()
{

}

Alien::~Alien()
{
	MV1DeleteModel(m_modelHandle);
}

void Alien::Init()
{
	//モデルの生成
	m_modelHandle = Model::Instance().CreatAlienModel();
	//アニメーションの初期化
	m_anim.Init(m_modelHandle,kIdleAnimName,true);
	//ステータスの初期化
	LoadStatus("Alien");
	//場所
	m_pos = kStartPos;
	//体の大きさ
	m_height = kHeight;
	m_collisionRadius = kCollisionRadius;
	//最初はカメラの方を向かせる
	m_forward = Vector3(0.0f, 0.0f, -1.0f);
	m_angle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	//m_gravity =
	MV1SetPosition(m_modelHandle, m_pos);

	//行動
	m_searchRange = kSearchRange;
	m_attackRange = kAttackRange;
	m_idleAnimName = kIdleAnimName;
	m_moveAnimName = kMoveAnimName;
	m_attackType = AttackType::Attack;

	ChangeState(std::make_unique<EnemyStateIdle>(*this));
}

void Alien::Update()
{
	//HPが0以下になったら死亡にする
	if (m_hp <= 0 && !m_isDead)
	{
		m_isDead = true;
		//TODO:死亡ステートを作る。今はその場で止まって行動しない
		StopMove();
	}

	//ゲーム全体の時間倍率に敵専用の倍率を掛ける(ウィッチタイム中は遅くなる)
	float timeScale = GetTimeScale();

	//ステートの更新
	if (!m_isDead)
	{
		UpdateState();
	}
	//アニメーションの更新
	m_anim.Update(timeScale);

	//移動(ウィッチタイム中は移動もゆっくりになる)
	m_pos += m_velocity * timeScale;

	//向いている方向に少しずつ体を向ける
	//モデルは-Z方向に向きを合わせる
	float targetAngle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	//目標の角度との差を-π~πにおさめる(遠回りして回転しないようにする)
	float diff = targetAngle - m_angle;
	while (diff > DX_PI_F)  diff -= DX_TWO_PI_F;
	while (diff < -DX_PI_F) diff += DX_TWO_PI_F;
	m_angle += diff * kRotateRate * timeScale;
}

void Alien::LateUpdate()
{
	//当たり判定で押し出された後の位置をモデルに反映する
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));
	MV1SetPosition(m_modelHandle, m_pos);
}

void Alien::Draw()
{
	MV1DrawModel(m_modelHandle);
}