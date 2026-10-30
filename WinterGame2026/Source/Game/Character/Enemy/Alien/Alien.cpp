#include "Alien.h"
#include "../../../../Common/Model.h"
#include "../State/EnemyStateIdle.h"
#include <cmath>

namespace
{
	const char* const kIdleAnimName = "Alien|Idle";
	const char* const kMoveAnimName = "Alien|Move";
	//被弾のアニメーション
	const char* const kDamageAnimName = "Alien|HitBody";
	//死亡のアニメーション(短い倒れ方の"Alien|Dead"もモデルに入っている)
	const char* const kDeathAnimName = "Alien|Death";
	//叩き落とされて倒れる・起き上がるアニメーション
	const char* const kKnockdownAnimName = "Alien|FlyingBack";
	const char* const kGetUpAnimName = "Alien|StandUp";
	//浮いている高さ
	constexpr float kHoverHeight = 90.0f;

	//最初の位置(プレイヤーと重ならないように少し奥に置く)
	const Vector3 kStartPos = Vector3(0.0f, kHoverHeight, 300.0f);
	//体の大きさ(デバッグ表示のカプセルを見ながら調整する)
	//身長
	constexpr float kHeight = 200.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 50.0f;

	//索敵範囲
	constexpr float kSearchRange = 1000.0f;
	//攻撃範囲
	constexpr float kAttackRange = 450.0f;
	//これより近づかれたら離れる
	constexpr float kTooCloseDistance = 250.0f;
	//攻撃の後にこの距離まで離れる
	constexpr float kKeepDistance = 400.0f;

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
	m_tooCloseDistance = kTooCloseDistance;
	m_keepDistance = kKeepDistance;
	m_idleAnimName = kIdleAnimName;
	m_moveAnimName = kMoveAnimName;
	m_damageAnimName = kDamageAnimName;
	m_deathAnimName = kDeathAnimName;
	m_knockdownAnimName = kKnockdownAnimName;
	m_getUpAnimName = kGetUpAnimName;
	m_attackType = AttackType::Breath;
	//浮いている高さ(叩き落とされたときに、この高さだけ地面へ落ちる)
	m_hoverHeight = kHoverHeight;

	ChangeState(std::make_unique<EnemyStateIdle>(*this));
}

void Alien::Update()
{
	//死亡演出が終わって消えた後は何もしない
	if (m_isVanished) return;

	//HPが0以下になったら死亡ステートにする
	CheckDeath();

	//ゲーム全体の時間倍率に敵専用の倍率を掛ける(ウィッチタイム中は遅くなる)
	float timeScale = GetTimeScale();

	//ステートの更新(死亡中も倒れる演出のために更新する)
	UpdateState();
	//アニメーションの更新倒れたまま待つ時は止める
	if (!m_isAnimPause)
	{
		m_anim.Update(timeScale);
	}	
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
	//消えた後は描画しない
	if (m_isVanished) return;

	//死亡演出でだんだん透明にする
	MV1SetOpacityRate(m_modelHandle, m_opacity);
	MV1DrawModel(m_modelHandle);
}