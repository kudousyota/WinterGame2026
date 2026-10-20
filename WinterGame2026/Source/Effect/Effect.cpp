#include "Effect.h"
#include "EffekseerForDXLib.h"

namespace
{
	//エフェクトの拡大率(1単位=1cmなのでEffekseer側の大きさに合わせて拡大する)
	constexpr float kScale = 20.0f;
}
void Effect::Play(int resourceHandle, const Vector3& pos, float rotY)
{
	m_resourceHandle = resourceHandle;
	m_pos = pos;
	m_playingHandle = PlayEffekseer3DEffect(resourceHandle);
	SetPosPlayingEffekseer3DEffect(m_playingHandle, m_pos.x, m_pos.y, m_pos.z);
	SetRotationPlayingEffekseer3DEffect(m_playingHandle, 0.0f, rotY, 0.0f);
	SetScalePlayingEffekseer3DEffect(m_playingHandle, kScale, kScale, kScale);
}

void Effect::Stop()
{
	//エフェクトの停止
	if (m_playingHandle != -1)
	{
		StopEffekseer3DEffect(m_playingHandle);
		m_playingHandle = -1;
	}
}

void Effect::Update()
{
	if (m_playingHandle == -1) return;

	//終了判定にする
	if (IsEffekseer3DEffectPlaying(m_playingHandle) != 1)
	{
		m_playingHandle = -1;
	}
}

bool Effect::IsPlaying() const
{
	//再生中のときだけtrue
	return (m_playingHandle != -1) && (IsEffekseer3DEffectPlaying(m_playingHandle) == 1);
}
