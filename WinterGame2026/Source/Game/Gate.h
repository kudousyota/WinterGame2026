#pragma once
#include "../DataLoader/SpawnTable.h"
#include <vector>

class Character;
class Gate
{
public:

	Gate() = default;
	Gate(const Gate&) = delete;
	Gate& operator=(const Gate&) = delete;
	~Gate();
	void Init(int maxNum);
	void Close(const std::vector<SpawnTable::GateInfo>& gates);
	void Open();
	//閉じるべきか(shouldClose)を毎フレーム受け取り、今の状態と違うときだけ開け閉めする
	//gates:閉じるときに置く場所
	void Update(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates);
	bool IsClosed()const;
	//閉じている壁だけ
	void ResolveWall(Character* character);

	void DrawDebug()const;


private:
	std::vector<int> m_handles;
	int m_activeNum = 0;
	//閉じているか
	//ゲートが0個のウェーブでも「閉じた」と覚えておくため、m_activeNumとは別に持つ
	bool m_isClosed = false;
};

