#pragma once
#include "../Math/Vector3.h"
class Effect
{
public:
	//rotYはY軸回転(ラジアン)。キャラクターの向きに合わせたいときに渡す
	void Play(int resourceHandle, const Vector3& pos, float rotY = 0.0f);

	void Stop();
	void Update();

	int GetPlayingHandle() const { return m_playingHandle; };
	int GetResourceHandle() const { return m_resourceHandle; }
	bool IsPlaying() const;
private:
	//エフェクトの再生位置
	Vector3 m_pos;
	//再生中のエフェクトハンドル
	int m_playingHandle = -1;
	//エフェクトのリソースハンドル
	int m_resourceHandle = -1;
};
