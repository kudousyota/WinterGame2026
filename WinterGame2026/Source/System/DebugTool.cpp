#include "DebugTool.h"
//デバッグ機能はDebugビルドでだけ使う
#ifdef _DEBUG
#include "DxLib.h"
#include "../Game/Character/Character.h"
#include "../DataLoader/DataManager.h"
#include "../Game/Camera.h"
#include "Input.h"
#include <iterator>
#include "../Color.h"

namespace
{
	//スロー中は何フレームに1回ゲームを更新するか
	constexpr int kSlowInterval = 4;
	//メッセージを表示するフレーム数
	constexpr int kMessageFrame = 120;
	//文字の行の高さ
	constexpr int kLineHeight = 16;

	//ボタン名をデバッグ表示用の日本語にする
	const char* GetButtonNameJp(const std::string& name)
	{
		if (name == "Jump")  return "ジャンプ";
		if (name == "Dodge") return "回避";
		if (name == "Punch") return "パンチ";
		if (name == "Kick")  return "キック";
		return name.c_str();
	}

	//メニューに出す名前(DebugTool::Showの順番に並べる)
	constexpr const char* kShowNames[] =
	{
		"文字情報",
		"グリッド",
		"当たり判定(体・ジャスト回避・ボスの弱点)",
		"攻撃判定(攻撃・ブレス)",
		"ロックオン範囲",
		"敵の範囲(索敵・攻撃・距離の円)",
		"敵の向き(視線の線)",
		"敵の攻撃枠マーク",
	};
	static_assert(std::size(kShowNames) == static_cast<int>(DebugTool::Show::Num), "kShowNamesの数をShowに合わせる");

	//メニューの位置と大きさ
	constexpr int kMenuX = 640;
	constexpr int kMenuY = 40;
	constexpr int kMenuWidth = 400;
	constexpr int kMenuPadding = 8;
}

DebugTool::DebugTool()
{
	//最初は全部表示する
	m_isShow.fill(true);
}

void DebugTool::Update(const std::vector<Target>& targets)
{
	//キーボードの状態を更新
	m_lastKeyState = m_keyState;
	GetHitKeyStateAll(m_keyState.data());

	//パッドのスティックの生の値
	GetJoypadAnalogInput(&m_leftStickX, &m_leftStickY, DX_INPUT_PAD1);
	GetJoypadAnalogInputRight(&m_rightStickX, &m_rightStickY, DX_INPUT_PAD1);

	//表示メニュー
	if (IsKeyTriggered(KEY_INPUT_TAB)) m_isMenuOpen = !m_isMenuOpen;
	if (m_isMenuOpen) UpdateMenu();

	//表示の切り替え(よく使うものはキー1つで切り替えられるようにしておく)
	if (IsKeyTriggered(KEY_INPUT_F1)) ToggleGroup({ Show::Collision, Show::AttackHit });
	if (IsKeyTriggered(KEY_INPUT_F2)) Toggle(Show::Text);
	if (IsKeyTriggered(KEY_INPUT_F3)) Toggle(Show::Grid);
	if (IsKeyTriggered(KEY_INPUT_F4)) Toggle(Show::LockOn);
	if (IsKeyTriggered(KEY_INPUT_F9)) ToggleGroup({ Show::EnemyRange, Show::EnemyForward, Show::EnemyToken });

	//CSVの読み直し
	if (IsKeyTriggered(KEY_INPUT_F5)) Reload(targets);
	//今いる位置をスポーン位置としてコピー
	if (IsKeyTriggered(KEY_INPUT_F11)) CopySpawnLine(targets);
	//敵を全部倒す
	if (IsKeyTriggered(KEY_INPUT_K)) KillEnemies(targets);

	//一時停止とスロー
	if (IsKeyTriggered(KEY_INPUT_F6)) m_isPaused = !m_isPaused;
	if (IsKeyTriggered(KEY_INPUT_F8))
	{
		m_isSlow = !m_isSlow;
		m_slowCounter = 0;
	}

	//このフレームにゲームを更新するか決める
	if (m_isMenuOpen)
	{
		//メニューの操作(矢印キーとEnter)がゲームに伝わらないように止める
		m_shouldUpdateGame = false;
	}
	else if (m_isPaused)
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
	if (IsShow(Show::Text)) DrawInfoText(targets, camera, frameCount);
	//メニューは文字情報を消していても出す
	if (m_isMenuOpen) DrawMenu();
}

void DebugTool::Toggle(Show type)
{
	auto& isShow = m_isShow[static_cast<int>(type)];
	isShow = !isShow;
}

void DebugTool::ToggleGroup(std::initializer_list<Show> types)
{
	//バラバラの状態でも1回でそろうように、1つ目に合わせる
	const bool next = !IsShow(*types.begin());
	for (Show type : types)
	{
		m_isShow[static_cast<int>(type)] = next;
	}
}

void DebugTool::UpdateMenu()
{
	constexpr int num = static_cast<int>(Show::Num);
	//上下で選ぶ(端まで行ったら反対側へ回る)
	if (IsKeyTriggered(KEY_INPUT_UP))   m_menuCursor = (m_menuCursor + num - 1) % num;
	if (IsKeyTriggered(KEY_INPUT_DOWN)) m_menuCursor = (m_menuCursor + 1) % num;
	//Enterか左右キーで切り替える
	if (IsKeyTriggered(KEY_INPUT_RETURN) || IsKeyTriggered(KEY_INPUT_LEFT) || IsKeyTriggered(KEY_INPUT_RIGHT))
	{
		Toggle(static_cast<Show>(m_menuCursor));
	}
}

void DebugTool::DrawMenu() const
{
	constexpr int num = static_cast<int>(Show::Num);

	//見出し + 項目 + 操作説明の行数
	const int height = kLineHeight * (num + 2) + kMenuPadding * 2;

	//後ろを暗くして読みやすくする
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(kMenuX, kMenuY, kMenuX + kMenuWidth, kMenuY + height, Color::kBlack, true);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(kMenuX, kMenuY, kMenuX + kMenuWidth, kMenuY + height, Color::kWhite, false);

	const int x = kMenuX + kMenuPadding;
	int y = kMenuY + kMenuPadding;
	DrawString(x, y, "デバッグ表示 (TABで閉じる・ゲーム停止中)", Color::kWhite);
	y += kLineHeight;

	for (int i = 0; i < num; i++)
	{
		const bool isSelected = (i == m_menuCursor);
		const bool isOn = m_isShow[i];
		//選んでいる項目は黄色、オフの項目は灰色
		const unsigned int color = isSelected ? Color::kYellow : (isOn ? Color::kWhite : Color::kGray);
		DrawFormatString(x, y, color, "%s [%s] %s", isSelected ? ">" : " ", isOn ? "ON " : "OFF", kShowNames[i]);
		y += kLineHeight;
	}

	DrawString(x, y, "↑↓:選択  Enter/←→:切り替え", Color::kGray);
}

void DebugTool::DrawInfoText(const std::vector<Target>& targets, const Camera& camera, int frameCount) const
{
	int y = 0;

	DrawString(0, y, "TAB:Menu F1:Collision F2:Text F3:Grid F4:LockOn F9:Enemy F5:Reload F6:Pause F7:Step F8:Slow F11:CopySpawnPos K:KillEnemies", Color::kWhite);
	y += kLineHeight;

	//フレーム数と、一時停止やスローの状態
	DrawFormatString(0, y, Color::kWhite, "FRAME:%d", frameCount);
	if (m_isPaused) DrawString(120, y, "PAUSE", Color::kYellow);
	if (m_isSlow) DrawFormatString(180, y, Color::kYellow, "SLOW x1/%d", kSlowInterval);
	y += kLineHeight;

	//キャラクターごとの情報
	for (const auto& target : targets)
	{
		const Character& c = *target.character;
		DrawFormatString(0, y, Color::kWhite, "%-7s %s", target.name.c_str(), c.GetDebugStateText().c_str());
		y += kLineHeight;
		const Vector3& pos = c.GetPos();
		DrawFormatString(0, y, Color::kWhite, "        HP:%d/%d%s  Pos:(%.0f, %.0f, %.0f)  Ground:%s",
			c.GetHp(), c.GetMaxHp(), c.IsDead() ? " (Dead)" : "",
			pos.x, pos.y, pos.z, c.IsGround() ? "Yes" : "No");
		y += kLineHeight;
	}

	//パッドのスティック(左は生の値と、デッドゾーン処理後にプレイヤーが使う値)
	auto& input = Input::Instance();
	DrawFormatString(0, y, Color::kWhite, "LStick  Raw:(%5d, %5d)  Input:(%.2f, %.2f)",
		m_leftStickX, m_leftStickY, input.GetStickLX(), input.GetStickLY());
	y += kLineHeight;
	DrawFormatString(0, y, Color::kWhite, "RStick  Raw:(%5d, %5d)  Value:(%.2f, %.2f)",
		m_rightStickX, m_rightStickY, input.GetStickRX(), input.GetStickRY());
	y += kLineHeight;
	//カメラ
	DrawFormatString(0, y, Color::kWhite, "Camera  Yaw:%.3f  Pitch:%.3f", camera.GetYaw(), camera.GetPitch());
	y += kLineHeight;
	//最後に使われた入力が何フレーム前に押されたか(黄色はバッファのおかげで出た入力)
	const auto& used = input.GetDebugUsedInput();
	if (used.age > 0)
	{
		DrawFormatString(0, y, Color::kYellow, "先行入力  %s：%dF前に押した入力を使用(バッファで成功)  [使ってから%dF経過]",
			GetButtonNameJp(used.name), used.age, input.GetNowFrame() - used.frame);
	}
	else if (used.age == 0)
	{
		DrawFormatString(0, y, Color::kWhite, "先行入力  %s：押したフレームにそのまま使用(バッファなしでも成功)  [使ってから%dF経過]",
			GetButtonNameJp(used.name), input.GetNowFrame() - used.frame);
	}
	else
	{
		DrawString(0, y, "先行入力  まだ記録なし", Color::kWhite);
	}
	y += kLineHeight;

	//読み直しの結果
	if (m_messageFrame > 0)
	{
		DrawString(0, y, m_message.c_str(), Color::kYellow);
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

void DebugTool::CopySpawnLine(const std::vector<Target>& targets)
{
	//一覧の先頭はプレイヤー
	if (targets.empty()) return;
	const Vector3& pos = targets[0].character->GetPos();

	//Spawn.csvの列: wave,type,base,spawnerX,spawnerZ,offsetX,offsetZ,delay
	//ウェーブ番号と敵の種類は仮に入れておくので、貼った後に書き換える
	char line[128];
	sprintf_s(line, "0,Goblin,world,%.0f,%.0f,0,0,0", pos.x, pos.z);
	SetClipboardText(line);

	m_message = std::string("Copied to clipboard: ") + line;
	m_messageFrame = kMessageFrame;
}

void DebugTool::KillEnemies(const std::vector<Target>& targets)
{
	int killCount = 0;
	//一覧の先頭はプレイヤーなので、2番目から(敵だけ)倒す
	for (size_t i = 1; i < targets.size(); i++)
	{
		Character* enemy = targets[i].character;
		//もう倒れている敵は数えない
		if (enemy->IsDead()) continue;

		//HPを0にするだけにして、死亡ステートへの切り替えは敵のUpdateにまかせる
		//(普通に倒したときと同じ流れになり、ウェーブも進む)
		enemy->Damage(enemy->GetHp());
		killCount++;
	}

	m_message = "Killed " + std::to_string(killCount) + " enemies";
	m_messageFrame = kMessageFrame;
}
#endif // _DEBUG
