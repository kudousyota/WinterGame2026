#include "GameManager.h"
#include "Character/Enemy/Alien/Alien.h"
#include "Character/Enemy/Warrok/Warrok.h"
#include "Character/Enemy/Goblin/Goblin.h"
#include "Character/Enemy/Dummy/Dummy.h"
#include "../System/StageCollision.h"
#include "../System/Timer.h"
#include "../Effect/EffectManager.h"
#include "../Game.h"
#include "DxLib.h"
#include <cassert>

namespace
{
	//ヒットストップとカメラシェイクの値は攻撃ごとにAttackHit.csvに書く
	//ウィッチタイム中に画面に重ねる色の濃さ
	constexpr int kWitchTimeOverlayAlpha = 128;

	//CSVの名前から敵を作る
	std::shared_ptr<Enemy> CreateEnemy(const std::string& name)
	{
		if (name == "Alien") return std::make_shared<Alien>();
		if (name == "Warrok") return std::make_shared<Warrok>();
		if (name == "Goblin") return std::make_shared<Goblin>();
		//チュートリアルのダミー//段階ごとに名前で動き方を変える
		if (name == "Dummy") return std::make_shared<Dummy>(Dummy::Mode::Stand);
		if (name == "DummyAttack") return std::make_shared<Dummy>(Dummy::Mode::Attack);
		if (name == "DummyFloat") return std::make_shared<Dummy>(Dummy::Mode::Float);
		assert(false && "知らない敵の名前");
		return nullptr;
	}
}

GameManager::GameManager() :
	m_battle(m_collision, m_projectiles)
{
}

void GameManager::Init(Stage::Type stageType, int maxGateNum)
{
	//ステージの生成
	m_pStage = std::make_shared<Stage>();
	m_pStage->Init(stageType);

	//先にプレイヤーを生成
	m_pPlayer = std::make_shared<Player>();
	m_pPlayer->Init();

	//ゲートの初期化
	m_gate.Init(maxGateNum);

	//射影//視野角と映る距離の設定もカメラが行う
	m_pCamera = std::make_shared<Camera>();
	//プレイヤーの背中側から見るように、正面の向きを渡す
	m_pCamera->Init(m_pPlayer->GetCameraTarget(), m_pPlayer->GetForward());
	//地形とのめり込み判定に使うステージを渡す
	m_pCamera->SetStageHandle(m_pStage->GetCollisionHandle());
}

bool GameManager::Update()
{
	//タイマーの更新
	//一時停止中やスロー中に進めると先行入力やヒットストップの時間がずれるからゲームが進むときだけ更新する
	Timer::Instance().Update();

	//ヒットストップ中はキャラクターを止めてカメラシェイクだけ動かす
	if (Timer::Instance().IsHitStop())
	{
		UpdateCamera();
		return false;
	}

	//キャラクターの更新から攻撃判定とカメラまで
	UpdateWorld();

	//カメラが確定してからEffekseerに同期して更新する
	EffectManager::Instns().Update();

	//このフレームの処理が全部終わってから消す
	RemoveVanishedEnemies();
	return true;
}

void GameManager::UpdateWorld()
{
	//当たり判定はこのフレームにいるキャラクターで行う//消えた敵はもう入っていない
	m_collision.BeginFrame(GetAllCharacters());
	//カメラ基準で移動させるためにカメラの向きを渡す
	//ロックオンの方向もカメラ基準だからロックオンより先に渡す
	//動く前の位置を覚えておく//速く動いて壁を飛び越えていないか、ここから線を飛ばして調べる
	for (Character* character : GetAllCharacters())
	{
		character->SavePrevPos();
	}
	m_pPlayer->SetCameraAxis(m_pCamera->GetForward(), m_pCamera->GetRight());
	//プレイヤーが攻撃の向きに使うからプレイヤーより先にロックオンを更新する
	//敵の一覧は作り直しになるので1回だけ作る
	auto enemies = GetEnemies();
	m_lockOn.Update(m_pPlayer->GetPos(), m_pCamera->GetForward(), enemies);
	m_pPlayer->SetAttackTarget(m_lockOn.GetAttackTarget(m_pPlayer->GetPos(), m_pPlayer->GetMoveInput(), m_pPlayer->GetForward(), enemies));
	m_pPlayer->Update();
	m_enemyManager.Update();

	//全員が動き終わってから、キャラクター同士の重なりをまとめて解消する
	m_collision.ResolveAll();

	//キャラクター同士の押し出しで壁にめり込むことがあるので、ステージとの判定は最後に行う
	for (Character* character : GetAllCharacters())
	{
		m_gate.ResolveWall(character);
		StageCollision::Resolve(character, m_pStage->GetCollisionHandle());
	}

	//押し出しで確定した位置をモデルに反映する
	m_pPlayer->LateUpdate();
	m_enemyManager.LateUpdate();
	//敵の位置が確定してからマーカーを合わせる
	m_lockOn.LateUpdate();

	//位置が確定してから攻撃判定と飛び道具の更新を行う
	m_battle.Update(GetAllCharacters());

	//カメラの揺れはカメラを動かす前に頼む
	//シーンが受け取らなかったフレームの分も消さずに足していく
	for (const BattleEvent& event : m_battle.TakeEvents())
	{
		if (event.type == BattleEvent::Type::Hit && event.shakeFrame > 0)
		{
			m_pCamera->Shake(event.shakePower, event.shakeFrame);
		}
		m_battleEvents.push_back(event);
	}

	UpdateCamera();
}

void GameManager::UpdateCamera()
{
	//ウィッチタイム中はカメラを引く
	m_pCamera->SetPull(Timer::Instance().IsEnemySlow());
	m_pCamera->Update(m_pPlayer->GetCameraTarget(), m_pPlayer->GetForward(), m_lockOn.GetTargetPos(), m_lockOn.GetTargetHeight().value_or(0.0f));
}

std::vector<BattleEvent> GameManager::TakeBattleEvents()
{
	//中身を渡してこれは空にする
	std::vector<BattleEvent> events;
	events.swap(m_battleEvents);
	return events;
}

void GameManager::RemoveVanishedEnemies()
{
	//当たり判定は毎フレーム一覧をもらうからここで外す必要はない
	m_enemyManager.RemoveVanished();
}

void GameManager::ClearEnemies()
{
	//当たり判定は毎フレーム一覧をもらいロックオンはweak_ptrで覚えているから消すだけ
	m_enemyManager.Clear();
}

void GameManager::SpawnEnemy(const std::string& name, const Vector3& groundPos)
{
	auto enemy = CreateEnemy(name);
	if (!enemy) return;

	//神殿の床のように高さが違う場所もあるから地面の高さに合わせる
	Vector3 pos = groundPos;
	float groundY = 0.0f;
	if (StageCollision::FindGroundY(m_pStage->GetCollisionHandle(), pos.x, pos.z, pos.y, groundY))
	{
		pos.y = groundY;
	}

	//Initで最初の位置が入るから位置はInitの後に決める
	enemy->Init();
	//出てきた場所までの間に壁があっても引っかからないようにWarpで置く
	enemy->Warp(pos + Vector3(0.0f, enemy->GetHoverHeight(), 0.0f));
	//最初のフレームから正しい位置に描かれるようモデルにも反映しておく
	enemy->LateUpdate();
	//敵はプレイヤーを狙う
	enemy->SetTarget(m_pPlayer.get());
	//攻撃の枠をもらうためにまとめて持っている人を教えておく
	enemy->SetManager(&m_enemyManager);

	//当たり判定には次のフレームのBeginFrameから入る
	m_enemyManager.Add(enemy);
}

void GameManager::UpdateGate(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates)
{
	m_gate.Update(shouldClose, gates);
}

void GameManager::Draw()
{
	//ステージは背景として扱うから紫を重ねる前に描く
	m_pStage->Draw();

	//ウィッチタイム中は背景を紫にする(キャラクターより先に描いてキャラクターには色を乗せない)
	if (Timer::Instance().IsEnemySlow())
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, kWitchTimeOverlayAlpha);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(125, 0, 185), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//プレイヤー、敵、飛び道具
	m_pPlayer->Draw();
	m_enemyManager.Draw();
	m_projectiles.Draw();

	//エフェクトの描画
	EffectManager::Instns().Draw();
}

#ifdef _DEBUG
void GameManager::DrawDebug(const DebugTool& debug) const
{
	using Show = DebugTool::Show;
	//キャラクターの当たり判定のカプセルを表示
	if (debug.IsShow(Show::Collision)) m_collision.DrawDebugBody(GetAllCharacters());
	//攻撃判定を表示//このフレームの攻撃とブレス
	if (debug.IsShow(Show::AttackHit))
	{
		m_collision.DrawDebugAttack();
		m_projectiles.DrawDebug();
	}
	//ロックオンできる範囲を表示
	if (debug.IsShow(Show::LockOn)) m_lockOn.DrawDebug(m_pPlayer->GetPos(), m_pCamera->GetForward());
	//敵の範囲、向き、攻撃枠を表示
	for (const Enemy* enemy : GetEnemyList())
	{
		if (enemy->IsVanished()) continue;
		if (debug.IsShow(Show::EnemyRange)) enemy->DrawDebugRange();
		if (debug.IsShow(Show::EnemyForward)) enemy->DrawDebugForward();
		if (debug.IsShow(Show::EnemyToken)) enemy->DrawDebugToken();
		//弱点も体の当たり判定と一緒に切り替える
		if (debug.IsShow(Show::Collision)) enemy->DrawDebugWeakPoint();
	}
}

std::vector<DebugTool::Target> GameManager::GetDebugTargets() const
{
	std::vector<DebugTool::Target> targets = { { "Player", m_pPlayer.get() } };
	//敵は何番目かを名前に付けて見分ける
	const auto& enemies = m_enemyManager.GetEnemies();
	for (size_t i = 0; i < enemies.size(); i++)
	{
		targets.push_back({ "Alien" + std::to_string(i), enemies[i].get() });
	}
	return targets;
}
#endif // _DEBUG

int GameManager::GetEnemyCount() const
{
	return static_cast<int>(m_enemyManager.GetEnemies().size());
}

bool GameManager::AreAllEnemiesVanished() const
{
	for (const Enemy* enemy : GetEnemyList())
	{
		if (!enemy->IsVanished()) return false;
	}
	return true;
}

bool GameManager::IsLockedOn() const
{
	//ロックオンしている敵がいればロックオン中
	return m_lockOn.GetTarget() != nullptr;
}

std::vector<Enemy*> GameManager::GetEnemyList() const
{
	std::vector<Enemy*> result;
	for (const auto& enemy : m_enemyManager.GetEnemies())
	{
		result.push_back(enemy.get());
	}
	return result;
}

std::vector<std::shared_ptr<Character>> GameManager::GetEnemies() const
{
	//shared_ptr<Enemy>からshared_ptr<Character>へは自動で変換される
	const auto& enemies = m_enemyManager.GetEnemies();
	return std::vector<std::shared_ptr<Character>>(enemies.begin(), enemies.end());
}

std::vector<Character*> GameManager::GetAllCharacters() const
{
	std::vector<Character*> result = { m_pPlayer.get() };
	for (Enemy* enemy : GetEnemyList())
	{
		result.push_back(enemy);
	}
	return result;
}
