#include "GameScene.h"
#include "DxLib.h"
#include "../System/Timer.h"
#include "../Game.h"
#include "../Effect/EffectManager.h"
#include "SceneController.h"
#include "GameClearScene.h"
#include "GameOverScene.h"
#include <algorithm>
#include "../DataLoader/DataManager.h"

namespace
{
	//ヒットストップとカメラシェイクの値は攻撃ごとにAttackHit.csvに書く
	//ウィッチタイム中に画面に重ねる色の濃さ
	constexpr int kWitchTimeOverlayAlpha = 128;
	//勝ち負けが決まってから画面を切り替えるまでのフレーム数
	constexpr int kGameEndWaitFrame = 120;
	//ジャスト回避の歪みが画面の端まで広がるフレーム数
	constexpr int kDistortionFrame = 40;
	//ジャスト回避の歪みの強さ
	constexpr float kDistortionStrength = 0.04f;
	//ジャスト回避の歪みの輪の太さ
	constexpr float kDistortionRingWidth = 0.2f;
	//ウィッチタイム中の揺らぎの強さ
	constexpr float kWaveStrength = 0.01f;
	//揺らぎが最大になるまでのフレーム数
	constexpr float kWaveFadeFrame = 20.0f;
	//1フレームで進める揺らぎの時間
	constexpr float kWaveTimeStep = 1.0f / 60.0f;
}

GameScene::GameScene(SceneController& controller):
Scene(controller),
m_frameCount(0)
{
}

GameScene::~GameScene()
{
	//エフェクトとスロー・ヒットストップは次のシーンに持ち越さない
	EffectManager::Instns().StopAll();
	Timer::Instance().Reset();
}

void GameScene::Init()
{
	//前のゲームのスロー・ヒットストップが残らないようにする
	Timer::Instance().Reset();

	//Zバッファの設定
	SetUseZBuffer3D(true);//Zバッファを使います
	SetWriteZBuffer3D(true);//描画する物体はZバッファにも距離を書き込む

	//ステージ、プレイヤー、カメラ、ゲートを用意する
	const auto& spawnTable = DataManager::GetInstance().GetSpawnTable();
	m_game.Init(spawnTable.GetMaxGateNum());

	//敵はプレイヤーを狙うのでプレイヤーの後に準備する
	m_spawner.Init(spawnTable);

	//画面の歪み
	m_distortion.Init();
}

void GameScene::Update()
{
#ifdef _DEBUG
	m_debug.Update(m_game.GetDebugTargets());
	//一時停止スロー中はゲームを進めない
	if (!m_debug.ShouldUpdateGame()) return;
#endif // _DEBUG

	//タイマーの更新
	//一時停止とスロー中に進めると先行入力やヒットストップの時間がずれるからゲームが進むときだけ更新する
	Timer::Instance().Update();

	//ヒットストップ中はキャラクターを止めてカメラシェイクだけ動かす
	if (Timer::Instance().IsHitStop())
	{
		m_game.UpdateCamera();
		UpdateDistortion();
		return;
	}

	m_frameCount++;

	//キャラクターの更新から攻撃判定とカメラまで
	m_game.Update();
	//敵が消える前に受け取る
	HandleBattleEvents();

	//カメラが確定してからEffekseerに同期して更新する
	EffectManager::Instns().Update();
	//歪みの中心はプレイヤーの画面上の位置でカメラが確定してから決める
	UpdateDistortion();

	//このフレームの処理が全部終わってから消す
	m_game.RemoveVanishedEnemies();
	//消えた敵を外した後だから残っている敵の数で次のウェーブを決められる
	UpdateSpawn();
	//勝ち負けの判定
	CheckGameEnd();
}

void GameScene::Draw()
{
	//ここからエフェクトまでを歪みの対象にする
	m_distortion.BeginCapture();

	//ステージは背景として扱うので、紫を重ねる前に描く
	m_game.DrawStage();

	//ウィッチタイム中は背景を紫にする(キャラクターより先に描いて、キャラクターには色を乗せない)
	if (Timer::Instance().IsEnemySlow())
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, kWitchTimeOverlayAlpha);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(125, 0, 185), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//プレイヤー、敵、飛び道具
	m_game.DrawObjects();
	//エフェクトの描画
	EffectManager::Instns().Draw();

	//歪ませて画面に出す
	m_distortion.EndCapture();
#ifdef _DEBUG
	//どれを出すかはDebugToolのメニューやFキーで切り替える
	if (m_debug.IsShow(DebugTool::Show::Grid)) DrawGrid();
	//当たり判定、ロックオン範囲、敵の範囲など
	m_game.DrawDebug(m_debug);
	//文字情報
	m_debug.Draw(m_game.GetDebugTargets(), m_game.GetCamera(), m_frameCount);
#endif //_DEBUG

}

void GameScene::HandleBattleEvents()
{
	//カメラの揺れはGameManagerが出しているのでここでは画面の演出だけ行う
	for (const BattleEvent& event : m_game.TakeBattleEvents())
	{
		switch (event.type)
		{
		case BattleEvent::Type::JustDodge:
			StartJustDodgeDistortion();
			break;
		case BattleEvent::Type::Hit:
			break;
		case BattleEvent::Type::Dodge:
			//今は見た目の演出はない//チュートリアルで回数を数えるのに使う
			break;
		}
	}
}

void GameScene::UpdateSpawn()
{
	//プレイヤー基準の敵はカメラの正面//画面の奥に出す
	const Camera& camera = m_game.GetCamera();
	SpawnContext spawnContext{ m_game.GetPlayer().GetPos(), camera.GetForward(), camera.GetRight() };
	m_spawner.Update(m_game.GetEnemyCount(), spawnContext);
	//戦闘はSpawnerのUpdateの中で始まって終わるのでその後にゲートを合わせる
	m_game.UpdateGate(m_spawner.IsBattleLocked(), m_spawner.GetBattleArea().gates);
	for (const auto& request : m_spawner.TakeSpawnRequests())
	{
		m_game.SpawnEnemy(request.characterName, request.groundPos);
	}
}

void GameScene::StartJustDodgeDistortion()
{
	//連続で成功したら最初から広げ直す
	m_distortionFrame = 0;
}

void GameScene::UpdateDistortion()
{
	ScreenDistortion::Param param = m_distortion.GetParam();

	//ウィッチタイム中は揺らぎを強くして、終わったら弱くする
	const float waveStep = kWaveStrength / kWaveFadeFrame;
	if (Timer::Instance().IsEnemySlow())
	{
		m_waveStrength = (std::min)(m_waveStrength + waveStep, kWaveStrength);
	}
	else
	{
		m_waveStrength = (std::max)(m_waveStrength - waveStep, 0.0f);
	}
	m_waveTime += kWaveTimeStep;
	param.time = m_waveTime;
	param.waveStrength = m_waveStrength;

	//揺らぎも中心からの距離を使うので、中心は毎フレーム決める
	VECTOR screenPos = ConvWorldPosToScreenPos(m_game.GetPlayer().GetCameraTarget().ToDxLibVector());
	param.centerX = screenPos.x / static_cast<float>(Game::kScreenWidth);
	param.centerY = screenPos.y / static_cast<float>(Game::kScreenHeight);

	//輪が出ていないときは輪の強さだけ0にする
	if (m_distortionFrame < 0)
	{
		param.strength = 0.0f;
		m_distortion.SetParam(param);
		return;
	}

	param.progress = static_cast<float>(m_distortionFrame) / static_cast<float>(kDistortionFrame);
	param.strength = kDistortionStrength;
	param.ringWidth = kDistortionRingWidth;
	m_distortion.SetParam(param);

	m_distortionFrame++;
	//広がりきったら終わり
	if (m_distortionFrame > kDistortionFrame)
	{
		m_distortionFrame = -1;
	}
}

void GameScene::CheckGameEnd()
{
	bool isGameOver = m_game.GetPlayer().IsDead();
	//最後のウェーブまで出し終わっていなければクリアではない
	//敵は倒れて消えるまで見せてからクリアにする
	bool isClear = m_spawner.IsAllWavesFinished() && m_game.AreAllEnemiesVanished();
	//まだ勝ち負けが決まっていない
	if (!isGameOver && !isClear) return;

	//倒れる様子を少し見せてから切り替える
	m_gameEndFrame++;
	if (m_gameEndFrame < kGameEndWaitFrame) return;

	//相打ちのときはゲームオーバーにする
	if (isGameOver)
	{
		m_controller.ChangeScene(std::make_shared<GameOverScene>(m_controller));
	}
	else
	{
		m_controller.ChangeScene(std::make_shared<GameClearScene>(m_controller));
	}
}

void GameScene::DrawGrid()
{
	//直線の始点と終点
	VECTOR startPos;
	VECTOR endPos;

	for (int z = -300; z <= 300; z += 100)
	{
		startPos = VGet(-300.0f, 0.0f, static_cast<float>(z));
		endPos = VGet(300.0f, 0.0f, static_cast<float>(z));
		DrawLine3D(startPos, endPos, 0xff0000);
	}
	for (int x = -300; x <= 300; x += 100)
	{
		startPos = VGet(static_cast<float>(x), 0.0f, -300.0f);
		endPos = VGet(static_cast<float>(x), 0.0f, 300.0f);
		DrawLine3D(startPos, endPos, 0x0000ff);
	}
}
