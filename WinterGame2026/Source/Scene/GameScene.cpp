#include "GameScene.h"
#include "DxLib.h"
#include "../Game/Character/Character.h"
#include "../Game/Character/Enemy/Alien/Alien.h"
#include "../Game/Damage.h"
#include "../System/Timer.h"
#include "../System/StageCollision.h"
#include "../Game.h"

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
	//カメラに映る一番遠い距離(ステージの端から端まで映るようにする)
	constexpr float kCameraFar = 7000.0f;
}

GameScene::GameScene():
m_frameCount(0)
{
}

GameScene::~GameScene()
{
}

void GameScene::Init()
{
	//Zバッファの設定
	SetUseZBuffer3D(true);//Zバッファを使います
	SetWriteZBuffer3D(true);//描画する物体はZバッファにも距離を書き込む

	//カメラの設定
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 300.0f, -700.0f), VGet(0.0f, 0.0f, 0.0f));
	SetupCamera_Perspective(DX_PI_F / 3.0f);
	SetCameraNearFar(200.0f, kCameraFar);

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

	m_pCamera = std::make_shared<Camera>();
	m_pCamera->Init(m_pPlayer->GetCameraTarget(),m_pPlayer->GetAngle());

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

	//ヒットストップ中はキャラクターを止めて、カメラ(シェイク)だけ動かす
	if (Timer::Instance().IsHitStop())
	{
		m_pCamera->Update(m_pPlayer->GetCameraTarget());
		return;
	}

	m_frameCount++;
	m_collision.BeginFrame();
	//カメラ基準で移動させるためにカメラの向きを渡す
	m_pPlayer->SetCameraAxis(m_pCamera->GetForward(), m_pCamera->GetRight());
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

	//飛び道具の更新
	m_projectiles.Update();

	m_pCamera->Update(m_pPlayer->GetCameraTarget());
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
#ifdef _DEBUG
	if (m_debug.IsShowGrid()) DrawGrid();
	//当たり判定のカプセルを表示
	if (m_debug.IsShowCollision()) m_collision.DrawDebug();
	//文字情報
	m_debug.Draw(GetDebugTargets(), *m_pCamera, m_frameCount);
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
			//確定した位置から判定の位置を求める
			Vector3 pos = attacker->GetAttackPos(request);

			for (const auto& hit : m_collision.CheckAttackSphere(request.data, pos))
			{
				ApplyAttackHit(*attacker, request, hit);
			}
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
	//(記録もしないので、無敵が切れた後にまだ判定が出ていれば当たる)
	if (!target.CanBeHit()) return;

	target.MarkAttackReceived(&attacker, request.attackId);
	target.OnHit(request.data, Damage::Calc(request.data, attacker, target));

	//ヒットストップ・カメラシェイク
	Timer::Instance().RequestHitStop(kHitStopFrame);
	m_pCamera->Shake(kShakePower, kShakeFrame);
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
