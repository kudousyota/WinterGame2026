#include "GameScene.h"
#include "DxLib.h"
#include "../Game/Character/Character.h"
#include "../Game/Character/Enemy/Alien/Alien.h"
#include "../Game/Damage.h"
#include "../System/Timer.h"
#include "../System/StageCollision.h"
#include "../Game.h"
#include "../Game/Bullet.h"
#include "../System/Input.h"
#include "../Effect/EffectManager.h"
#include "SceneController.h"
#include "GameClearScene.h"
#include "GameOverScene.h"

namespace
{
	//ヒットストップフレーム
	constexpr int kHitStopFrame = 5;
	//カメラシェイクの強さ
	constexpr float kShakePower = 8.0f;
	//カメラシェイクのフレーム数
	constexpr int kShakeFrame = 12;
	//ウィッチタイム中に画面に重ねる色の濃さ(0～255)
	constexpr int kWitchTimeOverlayAlpha = 128;
	//勝ち負けが決まってから画面を切り替えるまでのフレーム数(倒れる様子を見せる)
	constexpr int kGameEndWaitFrame = 120;
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

	//ステージの生成
	m_pStage = std::make_shared<Stage>();
	m_pStage->Init();

	//先にプレイヤーを生成
	m_pPlayer = std::make_shared<Player>();
	m_pPlayer->Init();

	//エイリアンの生成
	m_pAlien = std::make_shared<Alien>();
	m_pAlien->Init();
	//エイリアンはプレイヤーを狙う
	m_pAlien->SetTarget(m_pPlayer.get());

	//当たり判定に登録(登録・解除はシーンだけが行う)
	m_collision.Register(m_pPlayer.get());
	m_collision.Register(m_pAlien.get());

	//射影(視野角・映る距離)の設定もカメラが行う
	m_pCamera = std::make_shared<Camera>();
	m_pCamera->Init(m_pPlayer->GetCameraTarget(),m_pPlayer->GetAngle());
	//地形とのめり込み判定に使うステージを渡す
	m_pCamera->SetStageHandle(m_pStage->GetModelHandle());

}
void GameScene::Update()
{
#ifdef _DEBUG
	m_debug.Update(GetDebugTargets());
	//一時停止・スロー中はゲームを進めない(描画は続ける)
	if (!m_debug.ShouldUpdateGame()) return;
#endif // _DEBUG

	//タイマーの更新(時間スケールやウィッチタイムの残りフレームを減らす)
	//一時停止・スロー中に進めると、先行入力やヒットストップの時間がずれるので、ゲームが進むときだけ更新する
	Timer::Instance().Update();
	//ウィッチタイム中はカメラを引く
	m_pCamera->SetPull(Timer::Instance().IsEnemySlow());

	//ヒットストップ中はキャラクターを止めて、カメラ(シェイク)だけ動かす
	if (Timer::Instance().IsHitStop())
	{
		m_pCamera->Update(m_pPlayer->GetCameraTarget(),m_lockOn.GetTargetPos());
		return;
	}

	m_frameCount++;
	m_collision.BeginFrame();
	//カメラ基準で移動させるためにカメラの向きを渡す
	//(ロックオンの方向もカメラ基準なので、ロックオンより先に渡す)
	m_pPlayer->SetCameraAxis(m_pCamera->GetForward(), m_pCamera->GetRight());
	//プレイヤーが攻撃の向きに使うので、プレイヤーより先にロックオンを更新する
	UpdateLockOn();
	m_pPlayer->SetAttackTarget(GetAttackTarget());
	m_pPlayer->Update();
	m_pAlien->Update();

	//全員が動き終わってから、キャラクター同士の重なりをまとめて解消する
	m_collision.ResolveAll();

	//キャラクター同士の押し出しで壁にめり込むことがあるので、ステージとの判定は最後に行う
	StageCollision::Resolve(m_pPlayer.get(), m_pStage->GetModelHandle());
	StageCollision::Resolve(m_pAlien.get(), m_pStage->GetModelHandle());

	//押し出しで確定した位置をモデルに反映する
	m_pPlayer->LateUpdate();
	m_pAlien->LateUpdate();

	//位置が確定してから攻撃判定を行う
	ProcessAttacks();
	ProcessShots();
	//飛び道具の更新
	m_projectiles.Update();
	//飛び道具が動き終わってから当たり判定を行う
	ProcessProjectileHits();

	m_pCamera->Update(m_pPlayer->GetCameraTarget(),m_lockOn.GetTargetPos());
	//カメラが確定してからEffekseerに同期して更新する
	EffectManager::Instns().Update();

	CheckGameEnd();
}

void GameScene::Draw()
{
	//ステージは背景として扱うので、紫を重ねる前に描く
	m_pStage->Draw();

	//ウィッチタイム中は背景を紫にする(キャラクターより先に描いて、キャラクターには色を乗せない)
	if (Timer::Instance().IsEnemySlow())
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, kWitchTimeOverlayAlpha);
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(125, 0, 185), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	m_pPlayer->Draw();
	m_pAlien->Draw();
	//飛び道具の描画
	m_projectiles.Draw();
	//エフェクトの描画
	EffectManager::Instns().Draw();
#ifdef _DEBUG
	if (m_debug.IsShowGrid()) DrawGrid();
	//当たり判定のカプセルを表示
	if (m_debug.IsShowCollision()) m_collision.DrawDebug();
	//ロックオンできる範囲を表示
	if (m_debug.IsShowLockOn()) m_lockOn.DrawDebug(m_pPlayer->GetPos(), GetLockOnBaseDir());
	//敵の索敵・攻撃の範囲を表示
	if (m_debug.IsShowEnemyRange())
	{
		for (const Enemy* enemy : GetEnemyList())
		{
			if (!enemy->IsVanished()) enemy->DrawDebugRange();
		}
	}
	//文字情報
	m_debug.Draw(GetDebugTargets(), *m_pCamera, m_frameCount);
	if (auto pos = m_lockOn.GetTargetPos())
	{
		DrawSphere3D(VGet(pos->x, pos->y, pos->z), 15.0f, 8, 0xff0000, 0xff0000, true);
	}
#endif //_DEBUG

}

#ifdef _DEBUG
std::vector<DebugTool::Target> GameScene::GetDebugTargets() const
{
	return { { "Player", m_pPlayer.get() }, { "Alien", m_pAlien.get() } };
}
#endif // _DEBUG

void GameScene::ProcessAttacks()
{
	Character* characters[] = { m_pPlayer.get(), m_pAlien.get() };

	for (Character* attacker : characters)
	{
		for (const auto& request : attacker->TakeAttackRequests())
		{
			Vector3 pos = attacker->GetAttackPos(request);

			//lengthがあれば狙った方向へ伸びるカプセル、なければ球で判定する
			std::vector<CollisionManager::AttackHit> hits;
			if (request.length > 0.0f)
			{
				hits = m_collision.CheckAttackCapsule(request.data, pos, pos + request.dir * request.length);
			}
			else
			{
				hits = m_collision.CheckAttackSphere(request.data, pos);
			}

			for (const auto& hit : hits)
			{
				ApplyAttackHit(*attacker, request, hit);
			}
		}
	}
}

void GameScene::ProcessShots()
{
	Character* characters[] = { m_pPlayer.get(), m_pAlien.get() };

	for (Character* attacker : characters)
	{
		for (const auto& request : attacker->TakeShotRequests())
		{
			//確定した位置から判定の位置を求める
			Vector3 pos = attacker->GetShotPos(request);

			Vector3 forward = attacker->GetForward();
			m_projectiles.Add(std::make_unique<Bullet>(pos, forward, request.speed, request.data));
		}
	}
}

void GameScene::ProcessProjectileHits()
{
	for (const auto& projectile : m_projectiles.GetProjectiles())
	{
		//寿命などですでに消えている飛び道具は判定しない
		if (projectile->IsDead()) continue;

		const AttackData& data = projectile->GetAttackData();

		//速い弾がすり抜けないよう、前の位置から今の位置までをカプセルで判定する
		for (const auto& hit : m_collision.CheckAttackCapsule(data, projectile->GetPrevPos(), projectile->GetPos()))
		{
			Character& target = *hit.target;

			//ジャスト回避は無敵中でも成立する
			if (hit.isJustDodge)
			{
				target.OnJustDodge();
				projectile->Kill();
				break;
			}

			//無敵中・死亡中は受けない(弾は消さずに通り抜ける)
			if (!target.CanBeHit()) continue;

			//連射で何度も当たるので、ヒットストップとカメラシェイクは入れない
			target.OnHit(data, data.GetDamage());
			projectile->Kill();
			break;
		}
	}
}

void GameScene::ApplyAttackHit(Character& attacker, const AttackRequest& request, const CollisionManager::AttackHit& hit)
{
	Character& target = *hit.target;

	//同じ攻撃は1回しか受けない(判定が出ている間、毎フレーム当たらないようにする)
	if (target.HasReceivedAttack(&attacker, request.attackId)) return;

	//ジャスト回避は無敵中でも成立する
	if (hit.isJustDodge)
	{
		target.MarkAttackReceived(&attacker, request.attackId);
		target.OnJustDodge();
		return;
	}

	//無敵中・死亡中は受けない
	//記録しないから無敵が切れた後にまだ判定が出ていれば当たる
	if (!target.CanBeHit()) return;

	target.MarkAttackReceived(&attacker, request.attackId);
	target.OnHit(request.data, Damage::Calc(request.data, attacker, target));

	//ヒットストップ・カメラシェイク
	Timer::Instance().RequestHitStop(kHitStopFrame);
	m_pCamera->Shake(kShakePower, kShakeFrame);
}

Vector3 GameScene::GetLockOnBaseDir() const
{
	Vector3 dir = m_pPlayer->GetMoveInput();
	if (dir.SqMagnitude() <= 0.0f)
	{
		dir = m_pCamera->GetForward();
	}
	return dir;
}

void GameScene::UpdateLockOn()
{
	auto& input = Input::Instance();
	const Vector3& playerPos = m_pPlayer->GetPos();

	if (input.IsTriggered("target"))
	{
		//スティックを倒している方向の敵から狙う
		m_lockOn.Start(playerPos, GetLockOnBaseDir(), GetEnemies());
	}
	else if(!input.IsPressed("target"))
	{
		//ボタンをはなしたら外す
		m_lockOn.Release();
	}
	//死亡と距離のチェック
	m_lockOn.Update(playerPos);
}

const Character* GameScene::GetAttackTarget() const
{
	//ロックオン中はその敵
	if (m_lockOn.GetTargetPos())
	{
		return m_lockOn.GetTarget();
	}
	//ロックオンしていないときは、近くで一番狙いやすい敵を返す
	return m_lockOn.SearchSoft(m_pPlayer->GetPos(), m_pPlayer->GetMoveInput(), m_pPlayer->GetForward(), GetEnemies());
}

std::vector<Enemy*> GameScene::GetEnemyList()const
{
	return { m_pAlien.get() };
}

std::vector<Character*> GameScene::GetEnemies()const
{
	auto enemies = GetEnemyList();
	return std::vector<Character*>(enemies.begin(), enemies.end());
}

void GameScene::CheckGameEnd()
{
	bool isGameOver = m_pPlayer->IsDead();
	//敵は倒れて消えるまで見せてからクリアにする
	bool isClear = true;
	for (auto enemy : GetEnemyList())
	{
		if (!enemy->IsVanished())
		{
			isClear = false;
			break;
		}
	}
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
