#include "DebugTool.h"
//デバッグ機能はDebugビルドでだけ使う
#ifdef _DEBUG
#include "DxLib.h"
#include "../Game/Character/Character.h"
#include "../DataLoader/DataManager.h"
#include "../Game/Camera.h"
#include "Input.h"

namespace
{
	//スロー中は何フレームに1回ゲームを更新するか
	constexpr int kSlowInterval = 4;
	//メッセージを表示するフレーム数
	constexpr int kMessageFrame = 120;
	//文字の行の高さ
	constexpr int kLineHeight = 16;
}

DebugTool::DebugTool()
{
}

void DebugTool::Update(const std::vector<Target>& targets)
{
	//キーボードの状態を更新
	m_lastKeyState = m_keyState;
	GetHitKeyStateAll(m_keyState.data());

	//パッドのスティックの生の値
	GetJoypadAnalogInput(&m_leftStickX, &m_leftStickY, DX_INPUT_PAD1);
	GetJoypadAnalogInputRight(&m_rightStickX, &m_rightStickY, DX_INPUT_PAD1);

	//表示の切り替え
	if (IsKeyTriggered(KEY_INPUT_F1)) m_isShowCollision = !m_isShowCollision;
	if (IsKeyTriggered(KEY_INPUT_F2)) m_isShowText = !m_isShowText;
	if (IsKeyTriggered(KEY_INPUT_F3)) m_isShowGrid = !m_isShowGrid;

	//CSVの読み直し
	if (IsKeyTriggered(KEY_INPUT_F5)) Reload(targets);

	//一時停止・スロー
	if (IsKeyTriggered(KEY_INPUT_F6)) m_isPaused = !m_isPaused;
	if (IsKeyTriggered(KEY_INPUT_F8))
	{
		m_isSlow = !m_isSlow;
		m_slowCounter = 0;
	}

	//このフレームにゲームを更新するか決める
	if (m_isPaused)
	{
		//一時停止中はコマ送りのときだけ1フレーム進める
		m_shouldUpdateGame = IsKeyTriggered(KEY_INPUT_F7);
	}
	else if (m_isSlow)
	{
		m_shouldUpdateGame = (m_slowCounter % kSlowInterval == 0);
		m_slowCounter++;
	}
	else
	{
		m_shouldUpdateGame = true;
	}

	if (m_messageFrame > 0) m_messageFrame--;
}

void DebugTool::Draw(const std::vector<Target>& targets, const Camera& camera, int frameCount) const
{
	if (!m_isShowText) return;

	const unsigned int white = GetColor(255, 255, 255);
	const unsigned int yellow = GetColor(255, 255, 0);
	int y = 0;

	DrawString(0, y, "F1:Collision F2:Text F3:Grid F5:Reload F6:Pause F7:Step F8:Slow", white);
	y += kLineHeight;

	//フレーム数と一時停止・スローの状態
	DrawFormatString(0, y, white, "FRAME:%d", frameCount);
	if (m_isPaused) DrawString(120, y, "PAUSE", yellow);
	if (m_isSlow) DrawFormatString(180, y, yellow, "SLOW x1/%d", kSlowInterval);
	y += kLineHeight;

	//キャラクターごとの情報
	for (const auto& target : targets)
	{
		const Character& c = *target.character;
		DrawFormatString(0, y, white, "%-7s %s", target.name, c.GetDebugStateText().c_str());
		y += kLineHeight;
		const Vector3& pos = c.GetPos();
		DrawFormatString(0, y, white, "        HP:%d/%d%s  Pos:(%.0f, %.0f, %.0f)  Ground:%s",
			c.GetHp(), c.GetMaxHp(), c.IsDead() ? " (Dead)" : "",
			pos.x, pos.y, pos.z, c.IsGround() ? "Yes" : "No");
		y += kLineHeight;
	}

	//パッドのスティック(左は生の値と、デッドゾーン処理後にプレイヤーが使う値)
	auto& input = Input::Instance();
	DrawFormatString(0, y, white, "LStick  Raw:(%5d, %5d)  Input:(%.2f, %.2f)",
		m_leftStickX, m_leftStickY, input.GetStickLX(), input.GetStickLY());
	y += kLineHeight;
	DrawFormatString(0, y, white, "RStick  Raw:(%5d, %5d)", m_rightStickX, m_rightStickY);
	y += kLineHeight;
	//カメラ
	DrawFormatString(0, y, white, "Camera  AngleX:%.3f  AngleY:%.3f", camera.GetAngleX(), camera.GetAngleY());
	y += kLineHeight;

	//読み直しの結果
	if (m_messageFrame > 0)
	{
		DrawString(0, y, m_message.c_str(), yellow);
	}
}

bool DebugTool::IsKeyTriggered(int key) const
{
	return m_keyState[key] && !m_lastKeyState[key];
}

void DebugTool::Reload(const std::vector<Target>& targets)
{
	if (DataManager::GetInstance().ReloadData())
	{
		//最大HPや速度をキャラクターに反映する
		for (const auto& target : targets)
		{
			target.character->ReloadStatus();
		}
		m_message = "CSV reloaded";
	}
	else
	{
		m_message = "CSV reload failed (check the numbers in the CSV)";
	}
	m_messageFrame = kMessageFrame;
}
#endif // _DEBUG
