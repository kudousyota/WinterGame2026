#include "Input.h"
#include "Dxlib.h"


constexpr char kConfigfilename[] = "keyconf.kcnf";

namespace
{
	struct KeyConfHeader
	{
		//シグネチャ4バイト
		char signature[4];
		//バージョン番号4バイト
		float version;
		//データ数4バイト
		int dataNum;
		//12バイト
	};
}
Input& Input::Instance()
{
	static Input input;
	return input;
}

void Input::InitializeInputTable()
{
	m_inputTable["ok"] = { { PeripheralType::keyboard,KEY_INPUT_RETURN},
					  { PeripheralType::pad1,PAD_INPUT_A } };	// PADのAボタン
	m_inputTable["pause"] = { { PeripheralType::keyboard,KEY_INPUT_P},
					   { PeripheralType::pad1,PAD_INPUT_START } };	// PADのSTARTボタン

	m_inputTable["Attack"] = { { PeripheralType::keyboard,KEY_INPUT_Z},
						  { PeripheralType::pad1,PAD_INPUT_3 } };	// PADのYボタン
	m_inputTable["Jump"] = { { PeripheralType::keyboard,KEY_INPUT_LSHIFT},
						  { PeripheralType::pad1,PAD_INPUT_A } };	// PADのAボタン
	m_inputTable["Dodge"] = { { PeripheralType::keyboard,KEY_INPUT_X},
						{ PeripheralType::pad1,PAD_INPUT_X } };	// PADのXボタン

	m_inputTable["up"] = { { PeripheralType::keyboard,KEY_INPUT_UP},
						{ PeripheralType::pad1,PAD_INPUT_UP } };
	m_inputTable["down"] = { { PeripheralType::keyboard,KEY_INPUT_DOWN},
						  { PeripheralType::pad1,PAD_INPUT_DOWN } };
	m_inputTable["left"] = { { PeripheralType::keyboard,KEY_INPUT_LEFT},
						  { PeripheralType::pad1,PAD_INPUT_LEFT } };
	m_inputTable["right"] = { { PeripheralType::keyboard,KEY_INPUT_RIGHT},
						  { PeripheralType::pad1,PAD_INPUT_RIGHT } };
	m_inputTable["target"] = { { PeripheralType::keyboard,KEY_INPUT_SPACE},
							{ PeripheralType::pad1,PAD_INPUT_12 } };	// PADの右スティック押し込み
}



Input::Input() : m_inputData{}, m_lastInputData{}, m_inputTable{}
{
	InitializeInputTable();
	// 全イベントを false で初期化しておく（存在しないキーで at を呼ばないようにするため）
	for (const auto& kv : m_inputTable) {
		m_inputData[kv.first] = false;
		m_lastInputData[kv.first] = false;
	}
	Load();



}

void Input::Update()
{

	GetJoypadDirectInputState(DX_INPUT_PAD1, &m_padState);

	//まず現在の入力情報を取得
	char keyState[256];
	//生のキーボード情報
	GetHitKeyStateAll(keyState);
	//生のPad1を情報
	int padState = GetJoypadInputState(DX_INPUT_PAD1);
	//直前のフレームを更新(更新される前のフレーム情報をコピー)
	m_lastInputData = m_inputData;
	//全ての入力イベントをチェックする
	//ここでm_inputDataが更新される
	//m_inputTable回して各イベントの入力をチェックする
	for (const auto& inputInfo : m_inputTable)
	{
		auto& input = m_inputData[inputInfo.first];	// inputInfo.firstには"ok"等が入ってる
		// inputを書き換えると、inputData_のそのイベントが押されてるかどうかを着かえることになる
		// InputStateのvectorを回す
		for (const auto& state : inputInfo.second)
		{
			// このループをInputState配列のループなので、まず入力種別をチェックします
			switch (state.type)
			{
			case PeripheralType::keyboard:	//キーボードだったら
				//GetHitKeyStateAllで取ってきた配列の中身を見て入力されているかどうかチェック
				input = keyState[state.id];
				break;
			case PeripheralType::pad1:	//PADだったら
				//GetJoypadStateで取ってきたビット情報を見て登録されているビット情報と&を取り、そのビットが立っているかどうかをチェック
				input = (padState & state.id);
				break;
			}
			if (input)	//必須！
			{
				// ここでbreakしないと、最後のチェックで押されてないとfalseになる
				break;
			}
		}
	}
}



bool Input::IsPressed(const char* name)const
{
	// もし「ない」イベントを送られるとクラッシュします
	return m_inputData.at(name);	// const関数内部なので[]ではなくatを使用してる
}

bool Input::IsTriggered(const char* name) const
{
	return m_inputData.at(name) && !m_lastInputData.at(name);
}



void Input::Load()
{
	// 未実装: 設定ファイル読み込みは行わない
}



float Input::GetStickLX()const
{
	float value = m_padState.X / 1000.0f;

	if (fabs(value) < 0.2f)
	{
		value = 0.0f;
	}

	return value;
}

float Input::GetStickLY()const
{
	//判定
	float value = -m_padState.Y / 1000.0f;

	if (fabs(value) < 0.2f)
	{
		value = 0.0f;
	}

	return value;
}
