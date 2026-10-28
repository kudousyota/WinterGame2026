#include "EffectManager.h"
#include "EffekseerForDXLib.h"
#include "Effect.h"

//シングルトンインスタンスを取得
EffectManager& EffectManager::Instns()
{
    static EffectManager effect;
    return effect;
}

//再生中のエフェクトを停止し、ロードしたエフェクトリソースを解放する
void EffectManager::Terminate()
{
    //再生中のエフェクトを全て停止
    StopAll();

    //ロードしたエフェクトリソースを解放
    for (int i = 0; i < static_cast<int>(EffectType::Max); ++i)
    {
        int handle = m_resourceHandles[i];
        if (handle != -1)
        {
            DeleteEffekseerEffect(handle);
            m_resourceHandles[i] = -1;
        }
    }
}

void EffectManager::Init()
{
    //読み込みに失敗した種類があってもTerminateで誤って解放しないように-1で埋めておく
    m_resourceHandles.fill(-1);

    //攻撃が当たったときのエフェクト
    m_resourceHandles[static_cast<int>(EffectType::Hit)] = LoadEffekseerEffect("Data/Effect/Hit_Bayonetta.efk");
    //デカイ手のエフェクト
    m_resourceHandles[static_cast<int>(EffectType::WickedWeave)] = LoadEffekseerEffect("Data/Effect/WickedWeave.efk");
    //デカイ足のエフェクト
    m_resourceHandles[static_cast<int>(EffectType::WickedWeaveLeg)] = LoadEffekseerEffect("Data/Effect/WickedWeave_Leg.efk");
    //敵の攻撃の予兆エフェクト
    m_resourceHandles[static_cast<int>(EffectType::AttackTelegraph)] = LoadEffekseerEffect("Data/Effect/AttackTelegraph.efk");
    //デカイ足(踏みつけ)エフェクト
    m_resourceHandles[static_cast<int>(EffectType::WickedWeaveStomp)] = LoadEffekseerEffect("Data/Effect/WickedWeave_Stomp.efk");
}

void EffectManager::Update()
{
    //DXLibのカメラ・ライト情報をEffekseer側に同期
    Effekseer_Sync3DSetting();
    //再生中のエフェクトを更新し、再生終了したものはリストから削除
    //it = イテレータ
    for (auto it = m_effects.begin(); it != m_effects.end();)
    {
        (*it)->Update();

        if (!(*it)->IsPlaying())
        {
            //再生が終了していたらリストから削除
            it = m_effects.erase(it);
        }
        else
        {
            //再生中なら次の要素へ
            ++it;
        }
    }
    //Effekseerの3D更新処理
    UpdateEffekseer3D();
}

void EffectManager::Draw()
{
    //エフェクシアの3Dエフェクトを描画
    DrawEffekseer3D();
}

//指定した種類と座標でエフェクトを再生する
int EffectManager::PlayEffect(EffectType type, const Vector3& pos, const Vector3& rot, float scale)
{
    //指定したエフェクトのインスタンスを取得
    auto effect = std::make_unique<Effect>();
    //対応するリソースハンドルを使って指定座標で再生する
    effect->Play(GetResourceHandle(type), pos, rot, scale);
    //再生中のエフェクトのハンドルを返す
    int handle = effect->GetPlayingHandle();

    //管理リストに追加して更新・削除の対象にする
    m_effects.push_back(std::move(effect));

    return handle;
}

void EffectManager::StopEffect(int handle)
{
    for (auto it = m_effects.begin(); it != m_effects.end(); ++it)
    {
        if ((*it)->GetPlayingHandle() == handle)
        {
            (*it)->Stop();
            m_effects.erase(it);
            return;
        }
    }
}

void EffectManager::StopAll()
{
    for (auto& effect : m_effects)
    {
        if (effect)
        {
            //エフェクトの停止
            effect->Stop();
        }
    }
    //即時にリストからも削除する
    m_effects.clear();
}

//エフェクトの種類からリソースハンドルを取得
int EffectManager::GetResourceHandle(EffectType type) const
{
    return m_resourceHandles[static_cast<int>(type)];
}
