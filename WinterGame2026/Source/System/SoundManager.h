#pragma once
#include <array>
#include <vector>

enum class SoundType
{
	//BGM
	//一旦使いそうなやつを洗い出す
	BgmTitle,
	BgmGame,
	BgmClear,
	BgmGameOver,
	//SE
	Ok,
	Cursor,
	Dodge,
	JustDodge,
	WitchTimeStart,
	WitchTimeEnd,
	Hit,
	StrongHit,
	NoHit,
	Dead,
	Breath,
	Shout,
	Landing,
	Max

};

class SoundManager
{
public:
	static SoundManager& Instance();
	void Load();
	void Terminate();

	void PlaySE(SoundType type);
	void PlayBGM(SoundType type, bool isLoop = true);
	void StopBGM();

private:
	SoundManager() = default;
	~SoundManager() = default;
	SoundManager(const SoundManager&) = delete;
	SoundManager& operator=(const SoundManager&) = delete;

	//同じ音を重ねて鳴らすために複製したハンドルを持つ
	struct Sound
	{
		std::vector<int> handles;
		//次に鳴らすハンドルの番号
		int next = 0;
	};
	std::array<Sound, static_cast<int>(SoundType::Max)> m_sounds;
	//鳴っていないときはMax
	SoundType m_currentBgm = SoundType::Max;

};

