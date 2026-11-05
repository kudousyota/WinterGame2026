#include "Gate.h"
#include "../Common/Model.h"
#include <algorithm>
#include "../System/StageCollision.h"
#include "../Effect/EffectManager.h"

namespace
{
	//壁のエフェクトは1単位=1mで作っているので、cmに合わせて100倍にする
	constexpr float kWallEffectScale = 100.0f;
}

Gate::~Gate()
{
	for(int handle : m_handles)
	{
		MV1DeleteModel(handle);
	}
}

void Gate::Init(int maxNum)
{
	for(int i = 0; i < maxNum; i++)
	{
		int handle = Model::Instance().CreateGateModel();
		//当たり判定の情報は複製ごとに作る
		MV1SetupCollInfo(handle, -1, 1, 1, 1);
		m_handles.push_back(handle);
	}
}

void Gate::Close(const std::vector<SpawnTable::GateInfo>& gates)
{
	//使う数だけ有効にする
	int num = (std::min)(static_cast<int>(gates.size()), static_cast<int>(m_handles.size()));
	//一つずつ位置を合わせる
	for(int i = 0; i < num; i++)
	{
		//位置を合わせる処理をここに追加
		int handle = m_handles[i];
		MV1SetPosition(handle, gates[i].pos);
		MV1SetRotationXYZ(handle, Vector3(0.0f, gates[i].rotY, 0.0f));
		MV1RefreshCollInfo(handle, -1);

		//壁があることが見て分かるように、同じ場所にエフェクトを出す(壁と同じく下の中央が原点)
		int effect = EffectManager::Instns().PlayEffect(EffectType::GateWall, gates[i].pos, Vector3(0.0f, gates[i].rotY, 0.0f), kWallEffectScale);
		m_effectHandles.push_back(effect);
	}
	m_activeNum = num;
	m_isClosed = true;
}

void Gate::Open()
{
	//すべてのゲートを非表示にする
	for(int handle : m_handles)
	{
		MV1SetPosition(handle, Vector3(0.0f, -10000.0f, 0.0f));
	}
	//壁のエフェクトはフェードさせて消す
	for (int effect : m_effectHandles)
	{
		EffectManager::Instns().FadeOutEffect(effect);
	}
	m_effectHandles.clear();
	 m_activeNum = 0;
	 m_isClosed = false;
}

void Gate::Update(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates)
{
	//今の状態と同じなら何もしない(毎フレーム置き直すと当たり判定を作り直してしまう)
	if (shouldClose == m_isClosed) return;

	if (shouldClose)
	{
		Close(gates);
	}
	else
	{
		Open();
	}
}

bool Gate::IsClosed() const
{
	return m_isClosed;
}

void Gate::ResolveWall(Character* character)
{
	//閉じているゲートだけ判定する
	for (int i = 0; i < m_activeNum; i++)
	{
		//キャラクターと壁の衝突判定を行う
		StageCollision::CheckWall(character, m_handles[i]);
	}
}