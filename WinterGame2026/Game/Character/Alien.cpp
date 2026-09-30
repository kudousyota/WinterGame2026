#include "Alien.h"
#include "../../Comon/Model.h"

namespace
{
	const char* const kIdleAnimName = "Alien|Idle";
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
	//LoadStatus("Enemy");
	//場所
	m_pos = Vector3(0.0f, 0.0f, 0.0f);
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
	m_anim.Update(0.5f);

}

void Alien::Draw()
{
	MV1DrawModel(m_modelHandle);
}