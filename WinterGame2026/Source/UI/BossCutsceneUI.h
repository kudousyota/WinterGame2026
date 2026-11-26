#pragma once
#include "UIBase.h"
#include "../Game/BossCutscene.h"

class GameManager;

//ボスの登場と撃破の演出の見た目
//上下の黒帯、ボスの名前、とどめの白い光、「BOSS DEFEATED」
//いつ出すかはBossCutsceneの段階を読むだけで決める//このUIから演出は進めない
class BossCutsceneUI : public UIBase
{
public:
	//GameManagerはシーンが持っていてUIより長く残るので借りるだけにする
	explicit BossCutsceneUI(const GameManager& game);

	void Init()override;
	void Update(Input& input)override;
	void Draw()override;

private:
	//上下の黒帯
	void DrawLetterbox()const;
	//ボスの名前
	void DrawBossName()const;
	//とどめの白い光
	void DrawFlash()const;
	//「BOSS DEFEATED」
	void DrawDefeated()const;

	//演出の段階を読む//所有しない
	const GameManager* m_pGame = nullptr;
	//前のフレームの段階//変わった瞬間を見つけるのに使う
	BossCutscene::Phase m_prevPhase = BossCutscene::Phase::None;

	//黒帯が出ている割合//0で無し、1で出きった
	float m_letterboxRate = 0.0f;
	//名前の濃さ//0～1
	float m_nameRate = 0.0f;
	//名前を出し始めてからのフレーム数//出ていないときは-1
	int m_nameFrame = -1;
	//白い光が出てからのフレーム数//出ていないときは-1
	int m_flashFrame = -1;
	//「BOSS DEFEATED」を出してからのフレーム数//出ていないときは-1
	int m_defeatedFrame = -1;
};
