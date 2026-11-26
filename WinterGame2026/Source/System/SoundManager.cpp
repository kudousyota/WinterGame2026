#include "SoundManager.h"
#include <DxLib.h>
#include <cassert>
#include <iterator>
#include <string>

namespace
{
	struct SoundInfo
	{
		const char* path;
		//0から255
		int volume;
		//BGMはストリーミングで読み込む
		bool isBgm;
		//同時に鳴らせる数//BGMは複製できないので1
		int copyNum;
	};

	//並びはSoundTypeと同じにする
	//素材が見つかるまではファイルがなくても動くようにしている//置けばそのまま鳴る
	constexpr SoundInfo kSoundInfos[] =
	{
		//BGM
		{ "Data/Sound/BGM/Title.ogg",		100, true,	1 },
		{ "Data/Sound/BGM/Game.ogg",		100, true,	1 },
		{ "Data/Sound/BGM/Clear.ogg",		100, true,	1 },
		{ "Data/Sound/BGM/GameOver.ogg",	100, true,	1 },
		//SE
		{ "Data/Sound/SE/Ok.mp3",			170, false, 1 },
		{ "Data/Sound/SE/Cursor.mp3",		170, false, 2 },
		{ "Data/Sound/SE/Dodge.mp3",		170, false, 2 },
		{ "Data/Sound/SE/JustDodge.mp3",	170, false, 2 },
		{ "Data/Sound/SE/WitchTimeStart.mp3",170, false, 1 },
		{ "Data/Sound/SE/WitchTimeEnd.mp3",	170, false, 1 },
		{ "Data/Sound/SE/Hit.mp3",			170, false, 4 },
		{ "Data/Sound/SE/StrongHit.mp3",	170, false, 4 },
		{ "Data/Sound/SE/NoHit.mp3",		170, false, 2 },
		{ "Data/Sound/SE/Dead.mp3",			170, false, 3 },
		{ "Data/Sound/SE/Breath.mp3",		170, false, 1 },
		{ "Data/Sound/SE/Shout.mp3",		170, false, 1 },
		{ "Data/Sound/SE/Landing.mp3",		170, false, 2 },
	};
	static_assert(std::size(kSoundInfos) == static_cast<size_t>(SoundType::Max), "SoundTypeと表の数が合っていない");
}

SoundManager& SoundManager::Instance()
{
	static SoundManager instance;
	return instance;
}

void SoundManager::Load()
{
	for (int i = 0; i < static_cast<int>(SoundType::Max); ++i)
	{
		const SoundInfo& info = kSoundInfos[i];
		Sound& sound = m_sounds[i];

		//素材がまだないときは読み込まずに飛ばす//鳴らすときも何もしない
		if (FileRead_size(info.path) == -1)
		{
			OutputDebugStringA((std::string("[Sound] 見つからない: ") + info.path + "\n").c_str());
			continue;
		}

		//BGMは大きいのでファイルから少しずつ読みながら鳴らす
		SetCreateSoundDataType(info.isBgm ? DX_SOUNDDATATYPE_FILE : DX_SOUNDDATATYPE_MEMNOPRESS);
		const int handle = LoadSoundMem(info.path);
		//ファイルはあるのに読めないときは壊れているか形式が違う
		assert(handle != -1 && "サウンドの読み込みに失敗した");
		if (handle == -1) continue;

		ChangeVolumeSoundMem(info.volume, handle);
		sound.handles.push_back(handle);

		//同じ音を重ねて鳴らすために複製する//ストリーミングの音は複製できない
		if (info.isBgm) continue;
		for (int copy = 1; copy < info.copyNum; ++copy)
		{
			const int copyHandle = DuplicateSoundMem(handle);
			if (copyHandle == -1) break;
			ChangeVolumeSoundMem(info.volume, copyHandle);
			sound.handles.push_back(copyHandle);
		}
	}
	//ほかの所で読み込むときのために元に戻す
	SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMNOPRESS);
}

void SoundManager::Terminate()
{
	InitSoundMem();
	for (Sound& sound : m_sounds)
	{
		sound.handles.clear();
		sound.next = 0;
	}
	m_currentBgm = SoundType::Max;
}

void SoundManager::PlaySE(SoundType type)
{
	if (type == SoundType::Max) return;
	Sound& sound = m_sounds[static_cast<int>(type)];
	//読み込んでいない音は鳴らさない
	if (sound.handles.empty()) return;

	//複製したハンドルを順番に使う//全部鳴っていたら一番古い音を頭から鳴らし直す
	PlaySoundMem(sound.handles[sound.next], DX_PLAYTYPE_BACK);
	sound.next = (sound.next + 1) % static_cast<int>(sound.handles.size());
}

void SoundManager::PlayBGM(SoundType type, bool isLoop)
{
	if (type == SoundType::Max) return;
	const Sound& sound = m_sounds[static_cast<int>(type)];

	//同じ曲が鳴っているときは頭に戻さない
	if (type == m_currentBgm && !sound.handles.empty() && CheckSoundMem(sound.handles[0]) == 1) return;

	StopBGM();
	//読み込んでいない曲は鳴らさない//今の曲は止めておく
	if (sound.handles.empty()) return;

	PlaySoundMem(sound.handles[0], isLoop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
	m_currentBgm = type;
}

void SoundManager::StopBGM()
{
	if (m_currentBgm == SoundType::Max) return;
	const Sound& sound = m_sounds[static_cast<int>(m_currentBgm)];
	if (!sound.handles.empty())
	{
		StopSoundMem(sound.handles[0]);
	}
	m_currentBgm = SoundType::Max;
}
