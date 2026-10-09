#pragma once
#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "DxLib.h"
enum class PeripheralType
{
	keyboard,
	pad1,
};

struct InputState
{
	//入力された機種の種類
	PeripheralType type;
	//入力情報が入る(キーボードの場合はインデックス,パッドの場合はビット)
	int id;
};

class Input
{
public:
	static Input& Instance();

	Input();
	//これを毎フレーム呼び出す
	//入力情報を入れる
	void Update(int nowFrame);
	//特定のボタンが押されたかどうか
	bool IsPressed(const char* name)const;

	//特定のボタンが今押されたか(押された瞬間にしか反応しない)
	bool IsTriggered(const char* name)const;

	//void Save();
	void Load();

	float GetStickLX()const;
	float GetStickLY()const;

	//フレーム以内に押されたかどうか
	bool IsBuffered(const char* name, int frame)const;
	//フレーム以内に押されていたら、押されたフレーム番号を返す(なければ値なし)
	std::optional<int> GetBufferedFrame(const char* name, int frame)const;
	//使った入力をリセットする(2回反応しないようにする)
	void ConsumeBuffer(const char* name);

#ifdef _DEBUG
	//最後に使われた入力の記録(デバッグ表示用)
	struct UsedInputInfo
	{
		//ボタン名
		std::string name;
		//押してから使われるまでのフレーム数(-1はまだ記録なし)
		int age = -1;
		//使われたフレーム番号
		int frame = -1;
	};
	const UsedInputInfo& GetDebugUsedInput()const { return m_debugUsedInput; }
	//今のフレーム番号
	int GetNowFrame()const { return m_nowFrame; }
#endif

private:
	//イベント名と実際の入力の対応表
	std::map < std::string, std::vector<InputState>>m_inputTable;

	//実際に入力されたかどうかのデータ
	std::map<std::string, bool>m_inputData;
	//実際に入力されたかどうかのデータ
	std::map<std::string, bool>m_lastInputData;

	//コントローラーの入力情報
	DINPUT_JOYSTATE m_padState;

	//今のフレーム番号(ゲーム内の時計)
	int m_nowFrame = 0;

	//ボタンごとの最後に押されたフレーム番号
	//一度も押されていない(または使用済み)のボタンは値なし
	std::map<std::string, std::optional<int>> m_lastTriggeredFrame;

#ifdef _DEBUG
	//最後に使われた入力の記録
	UsedInputInfo m_debugUsedInput;
#endif

	void InitializeInputTable();
};

