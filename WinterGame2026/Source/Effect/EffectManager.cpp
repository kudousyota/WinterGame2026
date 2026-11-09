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
    //弾を撃つときの魔法陣
    m_resourceHandles[static_cast<int>(EffectType::SummonCircle)] = LoadEffekseerEffect("Data/Effect/Summon_MagicCircle.efk");
    //魔法の弾
    m_resourceHandles[static_cast<int>(EffectType::MagicBullet)] = LoadEffekseerEffect("Data/Effect/MagicBullet.efk");
    //ロックオンのマーカー
    m_resourceHandles[static_cast<int>(EffectType::LockOn)] = LoadEffekseerEffect("Data/Effect/LockOn.efk");
    //ウェーブ中に通れない壁
    m_resourceHandles[static_cast<int>(EffectType::GateWall)] = LoadEffekseerEffect("Data/Effect/GateWall.efk");
    //ボスの背中の弱点の目印
    m_resourceHandles[static_cast<int>(EffectType::WeakPoint)] = LoadEffekseerEffect("Data/Effect/WeakPoint.efk");
    //ボスの衝撃波
    m_resourceHandles[static_cast<int>(EffectType::Shockwave)] = LoadEffekseerEffect("Data/Effect/Shockwave.efk");
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

void EffectManager::SetEffectPos(int handle, const Vector3& pos)
{
    if (handle == -1) return;
    SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
}

void EffectManager::SetEffectScale(int handle, const Vector3& scale)
{
    if (handle == -1) return;
    SetScalePlayingEffekseer3DEffect(handle, scale.x, scale.y, scale.z);
}

void EffectManager::SetEffectAlpha(int handle, float alpha)
{
    if (handle == -1) return;
    //色はそのままで、不透明度だけ0~255に直して掛ける
    const int a = static_cast<int>(alpha * 255.0f);
    SetColorPlayingEffekseer3DEffect(handle, 255, 255, 255, (a < 0) ? 0 : (a > 255) ? 255 : a);
}

void EffectManager::FadeOutEffect(int handle)
{
    if (handle == -1) return;
    //一番上のノードだけ消すと、子はエフェクト側の「削除時」の設定でフェードして消える
    //消え終わったらUpdateでリストから外れる
    GetEffekseer3DManager()->StopRoot(handle);
}

//エフェクトの種類からリソースハンドルを取得
int EffectManager::GetResourceHandle(EffectType type) const
{
    return m_resourceHandles[static_cast<int>(type)];
}
