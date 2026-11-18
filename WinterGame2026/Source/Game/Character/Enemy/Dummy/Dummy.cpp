#include "Dummy.h"
#include "../../../../Common/Model.h"
#include "../State/EnemyStateIdle.h"
#include <cmath>
#include "../../../../Math/MathUtil.h"

namespace
{
	//ゴブリンのアニメーションを使い回す
	const char* const kIdleAnimName = "Goblin|Idle";
	const char* const kMoveAnimName = "Goblin|Walk";
	//被弾のアニメーション
	const char* const kDamageAnimName = "Goblin|Hit";
	//死亡のアニメーション//死なないので使わないが、空にはしない
	const char* const kDeathAnimName = "Goblin|Death";
	//吹っ飛ばされて倒れるアニメーション//叩き落とされたときもこれを使う
	const char* const kKnockdownAnimName = "Goblin|Knockback";

	//吹っ飛びのアニメーションで、体が宙に浮いて後ろへ飛んでいる区間の終わり
	constexpr float kKnockdownFlyEndRate = 0.7f;
	//地面に倒れきった割合
	constexpr float kKnockdownLieDownRate = 0.95f;

	//Floatモードで浮いている高さ//エイリアンと同じ
	constexpr float kFloatHoverHeight = 90.0f;

	//体の大きさ
	//身長
	constexpr float kHeight = 180.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 45.0f;

	//追いかけないので索敵範囲は0
	constexpr float kSearchRange = 0.0f;
	//攻撃範囲//この距離まで近づいたプレイヤーに攻撃する
	constexpr float kAttackRange = 130.0f;

	//回転の補完
	constexpr float kRotateRate = 0.15f;
}

Dummy::Dummy(Mode mode) :
	m_mode(mode)
{
}

Dummy::~Dummy()
{
	MV1DeleteModel(m_modelHandle);
}

void Dummy::Init()
{
	//モデルの生成
	m_modelHandle = Model::Instance().CreatGoblinModel();
	//アニメーションの初期化
	m_anim.Init(m_modelHandle, kIdleAnimName, true);
	//ステータスの初期化//移動速度は0にしてある
	LoadStatus("Dummy");
	//浮く高さ
	m_hoverHeight = (m_mode == Mode::Float) ? kFloatHoverHeight : 0.0f;
	//場所//出すときにSpawnEnemyで決め直される
	m_pos = Vector3(0.0f, m_hoverHeight, 0.0f);
	//体の大きさ
	m_height = kHeight;
	m_collisionRadius = kCollisionRadius;
	//最初はカメラの方を向かせる
	m_forward = Vector3(0.0f, 0.0f, -1.0f);
	m_angle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	MV1SetPosition(m_modelHandle, m_pos);

	//行動
	m_searchRange = kSearchRange;
	m_attackRange = kAttackRange;
	//近づかれても離れない//攻撃のあとも離れない
	m_tooCloseDistance = 0.0f;
	m_keepDistance = 0.0f;
	m_idleAnimName = kIdleAnimName;
	m_moveAnimName = kMoveAnimName;
	m_damageAnimName = kDamageAnimName;
	m_deathAnimName = kDeathAnimName;
	m_knockdownAnimName = kKnockdownAnimName;
	m_knockdownFlyEndRate = kKnockdownFlyEndRate;
	m_knockdownLieDownRate = kKnockdownLieDownRate;
	//攻撃の種類//フレームやダメージはcsvに書く
	m_attackType = AttackType::DummyAttack;

	ChangeState(std::make_unique<EnemyStateIdle>(*this));
}

void Dummy::Update()
{
	//死なないので死亡の確認はしない

	//ゲーム全体の時間倍率に敵専用の倍率を掛ける
	float timeScale = GetTimeScale();

	//ステートの更新
	UpdateState();
	//アニメーションの更新
	if (!m_isAnimPause)
	{
		m_anim.Update(timeScale);
	}
	//移動
	m_pos += m_velocity * timeScale;

	//向いている方向に少しずつ体を向ける
	//モデルは-Z方向に向きを合わせる
	float targetAngle = atan2f(m_forward.x, m_forward.z) + DX_PI_F;
	//目標の角度との差を-π~πにおさめる//遠回りして回転しないようにする
	float diff = MathUtil::WrapAngle(targetAngle - m_angle);
	m_angle += diff * kRotateRate * timeScale;
}

void Dummy::LateUpdate()
{
	//当たり判定で押し出された後の位置をモデルに反映する
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));
	MV1SetPosition(m_modelHandle, m_pos);
}

void Dummy::Draw()
{
	MV1SetOpacityRate(m_modelHandle, m_opacity);
	MV1DrawModel(m_modelHandle);
}

void Dummy::Damage(int damage)
{
	Character::Damage(damage);
	//死なない//0以下になったら最大まで戻す
	if (m_hp <= 0)
	{
		m_hp = m_maxhp;
	}
}

bool Dummy::CanStartAttack(float distance) const
{
	if (m_mode != Mode::Attack) return false;
	return Enemy::CanStartAttack(distance);
}

std::unique_ptr<StateBase> Dummy::CreateAfterAttackState()
{
	//速度が0なので、離れるステートや横に回るステートだと足踏みになってしまう
	return std::make_unique<EnemyStateIdle>(*this);
}
