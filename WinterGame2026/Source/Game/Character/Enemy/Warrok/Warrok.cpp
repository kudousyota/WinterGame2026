#include "Warrok.h"
#include "../../../../Common/Model.h"
#include "../State/EnemyStateIdle.h"
#include "../State/EnemyStateJumpAttack.h"
#include "../State/EnemyStateCharge.h"
#include "../../../../Effect/EffectManager.h"
#include <cmath>

namespace
{
	const char* const kIdleAnimName = "Warrok|Idle";
	const char* const kMoveAnimName = "Warrok|Run";
	//死亡のアニメーション
	const char* const kDeathAnimName = "Warrok|FlyingBackDeath";
	//被弾・叩き落とし・起き上がりのアニメーションはまだ無いので空のまま

	//地上の敵なので浮かない
	constexpr float kHoverHeight = 0.0f;

	//モデルの拡大率
	constexpr float kModelScale = 4.0f;

	//最初の位置
	const Vector3 kStartPos = Vector3(0.0f, kHoverHeight, 300.0f);
	//体の大きさ
	//身長
	constexpr float kHeight = 250.0f * kModelScale;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 60.0f * kModelScale;

	//距離はすべて体の半径を基準にする
	//プレイヤーは体に押し出されるので、中心から半径より内側には入ってこない
	//攻撃判定を出す位置
	constexpr float kHitForwardOffset = kCollisionRadius + 80.0f;
	//プレイヤーに当たる高さ
	constexpr float kHitHeight = 120.0f;

	//索敵範囲
	constexpr float kSearchRange = kCollisionRadius + 1100.0f;
	//攻撃範囲
	constexpr float kAttackRange = kCollisionRadius + 140.0f;
	//近づかれても離れない
	constexpr float kTooCloseDistance = 0.0f;
	//攻撃の後にこの距離まで離れる
	constexpr float kKeepDistance = kCollisionRadius + 260.0f;

	//回転の補完
	//ウィッチタイム中はこれにも時間の倍率がかかって遅くなる
	constexpr float kRotateRate = 0.15f;

	//弱点の位置
	//背中の上のほう(肩甲骨の間)のボーンを基準にする
	const char* const kWeakPointFrameName = "mixamorig:Spine2";
	//ボーンから背中の表面までの距離(Blenderで測ると約30cm)
	constexpr float kWeakPointBackOffset = 30.0f * kModelScale;
	//ボーンが見つからなかったときの位置
	const float kWeakPointOffset = kCollisionRadius;
	constexpr float kWeakPointHeight = kHeight * 0.5f;

	constexpr float kWeakPointRadius = 80.0f;

	//同じ攻撃はこの回数まで続けて出す(超えたらもう片方の攻撃にする)
	constexpr int kMaxSameAttackCount = 2;

	//ジャンプ攻撃
	//その場で跳んで衝撃波を出すので、衝撃波が届く距離にしておく
	constexpr float kJumpAttackRange = kCollisionRadius + 1000.0f;
	//一度使ったら、次に使えるまでのフレーム数
	//続けて跳びすぎないようにする
	constexpr float kJumpAttackCooldownFrame = 300.0f;

	//突進
	//突進で進む距離より少し短くして、届かない距離からは出さない
	constexpr float kChargeRange = kCollisionRadius + 1800.0f;
	//一度使ったら、次に使えるまでのフレーム数
	constexpr float kChargeCooldownFrame = 360.0f;

	//弱点に当たった時のダメージ倍率
	constexpr float kWeakPointDamageRate = 3.0f;
	//弱点以外に当たった時のダメージ倍率
	constexpr float kArmorDamageRate = 0.5f;

	//弱点の目印のエフェクト
	//拡大率(この大きさで広がるリングの半径がだいたい弱点の半径になる)
	constexpr float kWeakPointEffectScale = 22.0f;
	//弱点の中心からさらに後ろへずらす距離(体に半分埋もれて見えなくならないように)
	constexpr float kWeakPointEffectBackOffset = 40.0f;
}

Warrok::Warrok()
{
}

Warrok::~Warrok()
{
	//死ぬ前に消された(シーンが切り替わったなど)ときもエフェクトを残さない
	EffectManager::Instns().StopEffect(m_weakPointEffectHandle);
	MV1DeleteModel(m_modelHandle);
}

void Warrok::Init()
{
	//モデルの生成
	m_modelHandle = Model::Instance().CreatWarrokModel();
	//ボスなので大きくする
	MV1SetScale(m_modelHandle, VGet(kModelScale, kModelScale, kModelScale));
	//アニメーションの初期化
	m_anim.Init(m_modelHandle, kIdleAnimName, true);
	//ステータスの初期化
	LoadStatus("Warrok");
	//場所
	m_pos = kStartPos;
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
	m_tooCloseDistance = kTooCloseDistance;
	m_keepDistance = kKeepDistance;
	m_hitForwardOffset = kHitForwardOffset;
	m_hitHeight = kHitHeight;
	m_idleAnimName = kIdleAnimName;
	m_moveAnimName = kMoveAnimName;
	m_deathAnimName = kDeathAnimName;
	m_hoverHeight = kHoverHeight;
	//攻撃の種類
	m_attackType = AttackType::WarrokPunch;

	//弱点の基準のボーンを探して、最初の位置を決めておく
	m_weakPointFrame = MV1SearchFrame(m_modelHandle, kWeakPointFrameName);
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));
	if (m_weakPointFrame != -1)
	{
		//待機ポーズは上体がほぼまっすぐなので、ここで背中の表面の点を決める
		//ボーンは体の中にあるので、背中側へずらして体の表面に出す
		const Vector3 bonePos = MV1GetFramePosition(m_modelHandle, m_weakPointFrame);
		const Vector3 surfacePos = bonePos - GetModelForward() * kWeakPointBackOffset;
		//ボーンから見た位置に直して覚えておく
		const MATRIX frameMat = MV1GetFrameLocalWorldMatrix(m_modelHandle, m_weakPointFrame);
		m_weakPointLocalPos = VTransform(surfacePos, MInverse(frameMat));
	}
	UpdateWeakPointPos();

	//背中の弱点の目印(位置は出した後にLateUpdateで合わせる)
	m_weakPointEffectHandle = EffectManager::Instns().PlayEffect(EffectType::WeakPoint, GetWeakPointEffectPos(), Vector3(), kWeakPointEffectScale);

	ChangeState(std::make_unique<EnemyStateIdle>(*this));
}

void Warrok::Update()
{
	//死亡演出が終わって消えた後は何もしない
	if (m_isVanished) return;

	//HPが0以下になったら死亡ステートにする
	CheckDeath();
	//倒れたら弱点の目印は消す
	if (m_isDead)
	{
		StopWeakPointEffect();
	}

	//ゲーム全体の時間倍率に敵専用の倍率を掛ける
	float timeScale = GetTimeScale();

	//ジャンプ攻撃が使えるまでの時間を進める
	if (m_jumpAttackCooldown > 0.0f)
	{
		m_jumpAttackCooldown -= timeScale;
	}
	//突進が使えるまでの時間を進める
	if (m_chargeCooldown > 0.0f)
	{
		m_chargeCooldown -= timeScale;
	}
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
	//遠回りして回転しないようにする
	float diff = targetAngle - m_angle;
	while (diff > DX_PI_F)  diff -= DX_TWO_PI_F;
	while (diff < -DX_PI_F) diff += DX_TWO_PI_F;
	m_angle += diff * kRotateRate * timeScale;
}

void Warrok::LateUpdate()
{
	//当たり判定で押し出された後の位置をモデルに反映する
	MV1SetRotationXYZ(m_modelHandle, VGet(0.0f, m_angle, 0.0f));
	MV1SetPosition(m_modelHandle, m_pos);
	//モデルが動いた後のボーンの位置に弱点を合わせる
	UpdateWeakPointPos();
	//弱点の目印を背中についていかせる
	//(SetEffectPosは-1なら何もしないので、消した後も呼んで大丈夫)
	EffectManager::Instns().SetEffectPos(m_weakPointEffectHandle, GetWeakPointEffectPos());
}

void Warrok::Draw()
{
	//消えた後は描画しない
	if (m_isVanished) return;


	//死亡演出でだんだん透明にする
	MV1SetOpacityRate(m_modelHandle, m_opacity);
	MV1DrawModel(m_modelHandle);
}

#ifdef _DEBUG
void Warrok::DrawDebugWeakPoint() const
{
	DrawSphere3D(GetWeakPointPos(), kWeakPointRadius, 16, 0xff0000, 0xff0000, false);
}
#endif

float Warrok::GetDamageRate(const Vector3& start, const Vector3& end, float radius) const
{
	if (IsHitWeakPoint(start,end,radius))
	{
		return kWeakPointDamageRate;
	}
	else
	{
		return kArmorDamageRate;
	}

}

bool Warrok::CanStartAttack(float distance) const
{
	//殴れる距離なら攻撃する
	if (distance <= kAttackRange) return true;
	//離れていても、ジャンプ攻撃が使えて届くなら攻撃する
	if (m_jumpAttackCooldown <= 0.0f && distance <= kJumpAttackRange) return true;
	//突進が使えて届くなら攻撃する
	return m_chargeCooldown <= 0.0f && distance <= kChargeRange;
}

std::unique_ptr<StateBase> Warrok::CreateAttackState()
{
	if (m_attackType == AttackType::WarrokJumpAttack)
	{
		return std::make_unique<EnemyStateJumpAttack>(*this);
	}
	if (m_attackType == AttackType::WarrokCharge)
	{
		return std::make_unique<EnemyStateCharge>(*this);
	}
	return Enemy::CreateAttackState();
}

AttackType Warrok::SelectAttack()
{
	const float distance = GetTargetDistance();
	//殴れる距離ならパンチかスワイプ
	if (distance <= kAttackRange) return SelectMeleeAttack();

	//離れているときは、使える方の遠距離攻撃を選ぶ
	const bool canJump = m_jumpAttackCooldown <= 0.0f && distance <= kJumpAttackRange;
	const bool canCharge = m_chargeCooldown <= 0.0f && distance <= kChargeRange;

	//両方使えるときは半々で選ぶ
	if (canCharge && (!canJump || GetRand(1) == 0))
	{
		m_chargeCooldown = kChargeCooldownFrame;
		return AttackType::WarrokCharge;
	}
	//CanStartAttackが通った後に呼ばれるので、ここに来るのはジャンプ攻撃が使えるとき
	m_jumpAttackCooldown = kJumpAttackCooldownFrame;
	return AttackType::WarrokJumpAttack;
}

AttackType Warrok::SelectMeleeAttack()
{
	//前の近接攻撃のもう片方
	const AttackType other = (m_lastMeleeAttack == AttackType::WarrokPunch) ? AttackType::WarrokSwipe : AttackType::WarrokPunch;

	//同じ攻撃が続きすぎたらもう片方にする。それ以外は半々で選ぶ
	//GetRand(1)は0か1を返す
	AttackType next = m_lastMeleeAttack;
	if (m_sameAttackCount >= kMaxSameAttackCount || GetRand(1) == 0)
	{
		next = other;
	}

	//続けて出した回数を数える
	m_sameAttackCount = (next == m_lastMeleeAttack) ? m_sameAttackCount + 1 : 1;
	m_lastMeleeAttack = next;
	return next;
}

Vector3 Warrok::GetModelForward() const
{
	//モデルは-Z方向に向いているので、-Z方向のベクトルを回転させる
	return Vector3(-sinf(m_angle), 0.0f, -cosf(m_angle));
}

Vector3 Warrok::GetWeakPointPos() const
{
	return m_weakPointPos;
}

void Warrok::UpdateWeakPointPos()
{
	if (m_weakPointFrame == -1)
	{
		//ボーンが見つからないときは、足元から背中側へずらした位置にする
		const Vector3 backPos = m_pos - GetModelForward() * kWeakPointOffset;
		m_weakPointPos = Vector3(backPos.x, backPos.y + kWeakPointHeight, backPos.z);
		return;
	}
	//ボーンの今の位置と回転で、ボーンから見た位置をワールド座標に戻す
	const MATRIX frameMat = MV1GetFrameLocalWorldMatrix(m_modelHandle, m_weakPointFrame);
	m_weakPointPos = VTransform(m_weakPointLocalPos, frameMat);
}

bool Warrok::IsHitWeakPoint(const Vector3& start, const Vector3& end, float radius) const
{
	//弱点の中心位置を貰う
	const Vector3 CenterPos = GetWeakPointPos();
	const Vector3 closest = Geometry::ClosestPointOnSegment(start, end, CenterPos);

	const Vector3 diff = closest - CenterPos;

	const float dist2 = diff.SqMagnitude();

	const float hitRange = radius + kWeakPointRadius;

	return dist2 < hitRange * hitRange;
}

Vector3 Warrok::GetWeakPointEffectPos() const
{
	//当たり判定の中心は体の表面にあるので、見た目だけもう少し後ろに出す
	return GetWeakPointPos() - GetModelForward() * kWeakPointEffectBackOffset;
}

void Warrok::StopWeakPointEffect()
{
	//ぱっと消さずにエフェクト側のフェードアウトで消す
	EffectManager::Instns().FadeOutEffect(m_weakPointEffectHandle);
}
