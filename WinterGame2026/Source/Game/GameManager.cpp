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
#include <algorithm>
#include "../Color.h"

namespace
{
	//ヒットストップとカメラシェイクの値は攻撃ごとにAttackHit.csvに書く
	//ウィッチタイム中に背景へ重ねる色の濃さ//背景だけ少し沈めて、キャラクターを浮き立たせる
	//画面全体の色はシェーダーで暗い藍色と紫に変える
	constexpr int kWitchTimeOverlayAlpha = 120;
	//ウィッチタイム中に背景へ重ねる色はColor::kWitchTimeOverlay(黒に近い紫)
	//入ってから一番濃くなるまでのフレーム数//どこから色が変わるかは波紋が決めるので短くする
	constexpr float kWitchOverlayFadeInFrame = 6.0f;
	//終わってから元に戻るまでのフレーム数
	constexpr float kWitchOverlayFadeOutFrame = 20.0f;
	//ウィッチタイム中に画面の四隅を暗くする強さ
	constexpr float kWitchVignette = 0.8f;

	//ジャスト回避の歪みが画面の外まで広がるフレーム数//一瞬で画面全体に走るように短めにする
	constexpr int kDistortionFrame = 30;
	//波紋が広がりきったときの半径//DistortionPS.hlslの「progress * 2.2」と合わせる
	constexpr float kDistortionMaxRadius = 2.2f;
	//ジャスト回避の歪みの強さ
	constexpr float kDistortionStrength = 0.06f;
	//ジャスト回避の歪みの輪の太さ
	constexpr float kDistortionRingWidth = 0.22f;
	//ウィッチタイム中の揺らぎの強さ
	constexpr float kWaveStrength = 0.01f;
	//揺らぎが最大になるまでのフレーム数
	constexpr float kWaveFadeFrame = 20.0f;
	//1フレームで進める揺らぎの時間
	constexpr float kWaveTimeStep = 1.0f / 60.0f;

	//攻撃が当たったときのエフェクトの拡大率
	constexpr float kHitEffectScale = 30.0f;
	//弱点に当たったときのエフェクトの拡大率//ボスは大きいので普通のヒットより大きくする
	constexpr float kWeakPointHitEffectScale = 40.0f;
	//敵が出てくるときの魔法陣の拡大率//体の半径に掛ける//ボスのように大きい敵ほど大きく出す
	constexpr float kSpawnEffectScalePerRadius = 0.75f;
	//ジャスト回避が決まったときのエフェクトの拡大率
	constexpr float kJustDodgeEffectScale = 20.0f;

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

	//画面の歪み
	m_distortion.Init();
}

bool GameManager::Update()
{
	//タイマーの更新
	Timer::Instance().Update();

	//ヒットストップ中はキャラクターを止めてカメラシェイクと歪みの揺らぎだけ動かす
	if (Timer::Instance().IsHitStop())
	{
		UpdateCamera();
		UpdateDistortion();
		return false;
	}

	//ボスの演出を進める//ショットの切り替えをこのフレームのカメラに間に合わせるので先に進める
	m_bossCutscene.Update(*this);

	//キャラクターの更新から攻撃判定とカメラまで
	UpdateWorld();

	//ジャスト回避のイベントとカメラが決まってから歪みを進める
	UpdateDistortion();

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
	
	//動く前の位置を覚えておく//速く動いて壁を飛び越えていないか、ここから線を飛ばして調べる
	for (Character* character : GetAllCharacters())
	{
		character->SavePrevPos();
	}

	//カメラ基準で移動させるためにカメラの向きを渡す
	//ロックオンの方向もカメラ基準だからロックオンより先に渡す
	m_pPlayer->SetCameraAxis(m_pCamera->GetForward(), m_pCamera->GetRight());

	//プレイヤーが攻撃の向きに使うからプレイヤーより先にロックオンを更新する
	auto enemies = GetEnemies();
	//演出で操作を止めている間はロックオンのボタンも効かなくする
	m_lockOn.SetInputEnabled(!m_pPlayer->IsInputLocked());
	m_lockOn.Update(m_pPlayer->GetPos(), m_pCamera->GetForward(), enemies);
	m_pPlayer->SetAttackTarget(m_lockOn.GetAttackTarget(m_pPlayer->GetPos(), m_pPlayer->GetMoveInput(), m_pPlayer->GetForward(), enemies));
	m_pPlayer->Update();
	m_enemyManager.Update();

	//全員が動き終わってから、キャラクター同士の重なりをまとめて解消する
	m_collision.ResolveAll();

	//キャラクター同士の押し出しで壁にめり込むことがあるので、ステージとの判定は最後に行う
	for (Character* character : GetAllCharacters())
	{
		//ゲートとステージのどちらの壁に当たっても壁に当たったことにする
		//両方から押し出したいので、||でつながずに別々に呼ぶ
		const bool isHitGate = m_gate.ResolveWall(*character);
		const bool isHitStage = StageCollision::Resolve(*character, m_pStage->GetCollisionHandle());
		//次のフレームのUpdateで、ステートがこの結果を見る
		character->SetHitWall(isHitGate || isHitStage);
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
		//当たった所にヒットのエフェクトを出す//無敵などで受けなかったときは出さない
		if (event.type == BattleEvent::Type::Hit && event.result != HitResult::Ignored)
		{
			EffectManager::Instns().PlayEffect(EffectType::Hit, event.hitPos, Vector3(), kHitEffectScale);
			//弱点なら金色の破裂も重ねる//3倍ダメージが入ったとわかるようにする
			if (event.isWeakPoint)
			{
				EffectManager::Instns().PlayEffect(EffectType::WeakPointHit, event.hitPos, Vector3(), kWeakPointHitEffectScale);
			}
		}
		//ジャスト回避の歪みを始める//連続で成功したら最初から広げ直す
		if (event.type == BattleEvent::Type::JustDodge)
		{
			m_distortionFrame = 0;
			//まだウィッチタイムの色になっていなければ、波紋が通ったところから色を変える
			//ウィッチタイム中にもう一度決めたときは、もう全体が色づいているので広げ直さない
			m_isGradeSweeping = (m_witchOverlayRate < 0.5f);
			//よけた人の体の真ん中で光らせる//歪みと一緒に出して成功したとはっきりわかるようにする
			if (event.target)
			{
				EffectManager::Instns().PlayEffect(EffectType::JustDodge, event.target->GetCenterPos(), Vector3(), kJustDodgeEffectScale);
			}
		}
		m_battleEvents.push_back(event);
	}

	//とどめが入ったフレームのうちに撃破の演出を始める
	//次のフレームまで待つと、とどめのヒットストップが終わってからもう一度止まってしまう
	m_bossCutscene.CheckDefeat(*this);

	UpdateCamera();
}

void GameManager::UpdateCamera()
{
	//演出中は普段のカメラを動かさず、演出用のカメラのポーズを映す
	//揺れはCameraのApplyで足すので、演出中も揺れる
	if (m_eventCamera.IsActive())
	{
		m_eventCamera.Update();
		m_pCamera->Apply(m_eventCamera.GetPose());
		return;
	}

	//ウィッチタイム中はカメラを引く
	m_pCamera->SetPull(Timer::Instance().IsEnemySlow());
	m_pCamera->Update(m_pPlayer->GetCameraTarget(), m_pPlayer->GetForward(), m_lockOn.GetTargetPos(), m_lockOn.GetTargetHeight().value_or(0.0f));
}

void GameManager::StartEventCamera(const EventShot& shot)
{
	//普段のカメラが今映しているところから寄せ始める
	m_eventCamera.Start(m_pCamera->GetPose(), shot);
}

void GameManager::SetEventShot(const EventShot& shot)
{
	m_eventCamera.SetShot(shot);
}

void GameManager::EndEventCamera()
{
	if (!m_eventCamera.IsActive()) return;
	//最後に映していたところから普段のカメラを作り直す
	m_pCamera->SyncFromPose(m_eventCamera.GetPose());
	m_eventCamera.End();
}

void GameManager::UpdateDistortion()
{
	ScreenDistortion::Param param = m_distortion.GetParam();

	//ウィッチタイム中は揺らぎを強くして終わったら弱くする
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

	//背景の暗い紫も揺らぎと同じように、入るときは濃くして終わったら薄くする
	if (Timer::Instance().IsEnemySlow())
	{
		m_witchOverlayRate = (std::min)(m_witchOverlayRate + 1.0f / kWitchOverlayFadeInFrame, 1.0f);
	}
	else
	{
		m_witchOverlayRate = (std::max)(m_witchOverlayRate - 1.0f / kWitchOverlayFadeOutFrame, 0.0f);
	}

	//揺らぎも中心からの距離を使うので、中心は毎フレーム決める
	VECTOR screenPos = ConvWorldPosToScreenPos(m_pPlayer->GetCameraTarget().ToDxLibVector());
	param.centerX = screenPos.x / static_cast<float>(Game::kScreenWidth);
	param.centerY = screenPos.y / static_cast<float>(Game::kScreenHeight);

	//画面全体の色と四隅の暗さは背景の紫と同じ割合で変える
	param.gradeAmount = m_witchOverlayRate;
	param.vignette = kWitchVignette * m_witchOverlayRate;
	//色を変える範囲//波紋が広がっている間は波紋の内側だけ、それ以外は画面全体
	param.gradeRadius = 100.0f;

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
	if (m_isGradeSweeping)
	{
		param.gradeRadius = param.progress * kDistortionMaxRadius;
	}
	m_distortion.SetParam(param);

	m_distortionFrame++;
	//広がりきったら終わり
	if (m_distortionFrame > kDistortionFrame)
	{
		m_distortionFrame = -1;
		m_isGradeSweeping = false;
	}
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

	//足元に魔法陣と光の柱を出す//浮いている敵も魔法陣は地面に置く
	EffectManager::Instns().PlayEffect(EffectType::EnemySpawn, pos, Vector3(), enemy->GetCollisionRadius() * kSpawnEffectScalePerRadius);
	//柱の中で姿を隠し、柱が消えるときに立っているように見せる//狙う相手を預かるのでSetTargetの後
	enemy->StartAppear();

	//当たり判定には次のフレームのBeginFrameから入る
	m_enemyManager.Add(enemy);

	//ボスなら登場の演出を流す//光の柱から出てくるところから映す
	if (enemy->IsBoss())
	{
		m_bossCutscene.StartIntro(*this, enemy);
	}
}

void GameManager::UpdateGate(bool shouldClose, const std::vector<SpawnTable::GateInfo>& gates)
{
	m_gate.Update(shouldClose, gates);
}

void GameManager::Draw()
{
	//ここからエフェクトまでを歪みの対象にする
	m_distortion.BeginCapture();

	//ステージは背景として扱うから紫を重ねる前に描く
	m_pStage->Draw();

	//ウィッチタイム中はここまで(ステージ)だけを暗い藍色と紫にする
	//プレイヤーと敵はこの後に描くので色は変わらない
	m_distortion.ApplyGrade();

	//ウィッチタイム中は背景を黒に近い紫に沈める(キャラクターより先に描いてキャラクターには色を乗せない)
	//入るときと終わるときはだんだん変える
	const int overlayAlpha = static_cast<int>(kWitchTimeOverlayAlpha * m_witchOverlayRate);
	if (overlayAlpha > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, overlayAlpha);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, Color::kWitchTimeOverlay, true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	//プレイヤー、敵、飛び道具
	m_pPlayer->Draw();
	m_enemyManager.Draw();
	m_projectiles.Draw();

	//エフェクトの描画
	EffectManager::Instns().Draw();

	//歪ませて画面に出す
	m_distortion.EndCapture();
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
