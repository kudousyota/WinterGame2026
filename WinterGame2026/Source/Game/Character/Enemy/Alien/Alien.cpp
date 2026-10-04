#include "Alien.h"
#include "../../../../Common/Model.h"
#include "../../../../System/Timer.h"

namespace
{
	const char* const kIdleAnimName = "Alien|Idle";

	//最初の位置(プレイヤーと重ならないように少し奥に置く)
	const Vector3 kStartPos = Vector3(0.0f, 0.0f, 300.0f);

	//体の大きさ(デバッグ表示のカプセルを見ながら調整する)
	//身長
	constexpr float kHeight = 200.0f;
	//当たり判定のカプセルの半径
	constexpr float kCollisionRadius = 50.0f;
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
	//最初はカメラの方向かせる
	m_forward = Vector3(0.0f, 0.0f, 0.0f);
	//m_gravity = 
	MV1SetPosition(m_modelHandle, m_pos);

}

void Alien::Update()
{
	//HPが0以下になったら強制的に死亡ステートにする
	if (m_hp <= 0 && !m_isDead)
	{
		m_isDead = true;
	}

	//ステートの更新

	//アニメーションの更新
	//ゲーム全体の時間倍率に敵専用の倍率を掛ける(ジャスト回避中は遅くなる)
	float timeScale = Timer::Instance().GetTimeScale() * Timer::Instance().GetEnemyTimeScale();
	m_anim.Update(timeScale);

}

void Alien::LateUpdate()
{
	//当たり判定で押し出された後の位置をモデルに反映する
	MV1SetPosition(m_modelHandle, m_pos);
}

void Alien::Draw()
{
	MV1DrawModel(m_modelHandle);
}