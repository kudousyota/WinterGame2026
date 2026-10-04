#pragma once
#include <array>
#include <map>
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
	void Update();
	//特定のボタンが押されたかどうか
	bool IsPressed(const char* name)const;

	//特定のボタンが今押されたか(押された瞬間にしか反応しない)
	bool IsTriggered(const char* name)const;

	//void Save();
	void Load();

	float GetStickLX()const;
	float GetStickLY()const;


private:
	//イベント名と実際の入力の対応表
	std::map < std::string, std::vector<InputState>>m_inputTable;

	//実際に入力されたかどうかのデータ
	std::map<std::string, bool>m_inputData;
	//実際に入力されたかどうかのデータ
	std::map<std::string, bool>m_lastInputData;

	//コントローラーの入力情報
	DINPUT_JOYSTATE m_padState;

	void InitializeInputTable();
};

