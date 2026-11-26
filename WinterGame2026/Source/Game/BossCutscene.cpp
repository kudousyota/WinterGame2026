#include "BossCutscene.h"
#include "GameManager.h"
#include "Character/Enemy/Enemy.h"
#include "../System/Input.h"
#include "../System/Timer.h"

namespace
{
	//登場：引きのショット//床すれすれから全身を見上げる
	//カメラを低く、見る点を高くするほど見上げる角度が強くなって大きく見える
	constexpr float kWideDistanceRate = 1.6f;
	constexpr float kWideHeightRate = 0.06f;
	constexpr float kWideLookRate = 0.55f;
	//引きのショットへ寄る速さ(0～1)
	constexpr float kWideFollowRate = 0.06f;

	//登場：叫ぶところを足元近くから見上げるショット
	//叫ぶときは前のめりになって頭が低く手前に出るので、身長の割合ではなく頭の骨を追って見る
	//カメラの位置(身長に対する割合)//前に出てくる頭にぶつからないよう少し離す
	constexpr float kCloseDistanceRate = 1.1f;
	constexpr float kCloseHeightRate = 0.1f;
	//見る点//体の真ん中から顔までのどこを見るか(0で体の真ん中、1で顔)
	//顔だけを見ると体が画面の下に切れるので、少し体寄りにする
	constexpr float kCloseFaceRate = 0.75f;
	//寄る速さと頭を追う速さ(0～1)//ゆっくり追うので、頭が下がると遅れてあおる動きになる
	constexpr float kCloseFollowRate = 0.05f;

	//叫び始めてから揺らすまでのフレーム数//Screamの37～55fが叫びのピーク
	constexpr int kRoarShakeFrame = 37;
	//叫びの揺れ//ウィケッドウィーブ(16,20)より少し弱く、長めに揺らす
	constexpr float kRoarShakePower = 12.0f;
	constexpr int kRoarShakeLength = 40;
	//叫びが終わらなかったときに先へ進むフレーム数(アニメーションが見つからないときの保険)
	constexpr int kRoarMaxFrame = 180;

	//登場が始まってから飛ばせるようになるまでのフレーム数
	//決定(A)はジャンプと同じボタンなので、出てきた瞬間のジャンプで飛ばしてしまわないようにする
	constexpr int kSkipStartFrame = 30;

	//プレイヤーの後ろへ戻る速さ(0～1)
	constexpr float kReturnFollowRate = 0.08f;
	//戻りきらなくても終えるフレーム数
	constexpr int kReturnMaxFrame = 60;

	//撃破：とどめで止める長さ//コンボの締め(8～12)より長くして、決着がついたとわかるようにする
	constexpr int kDefeatHitStopFrame = 24;
	//とどめの揺れ
	constexpr float kDefeatShakePower = 20.0f;
	constexpr int kDefeatShakeLength = 30;
	//ヒットストップのあとのスロー//全体の時間を遅くするので、倒れるアニメーションもゆっくりになる
	constexpr float kDefeatSlowScale = 0.25f;
	constexpr int kDefeatSlowFrame = 90;
	//撃破：横から映すショット
	constexpr float kDefeatDistanceRate = 2.0f;
	constexpr float kDefeatHeightRate = 0.35f;
	constexpr float kDefeatLookRate = 0.35f;
	//横からのショットへ寄る速さ(0～1)//ヒットストップの間に回り込みきる
	constexpr float kDefeatFollowRate = 0.12f;
	//消えてから演出を終えるまで//この後シーンがクリア画面へ切り替えるまで待つ間も文字は出したまま
	constexpr int kDefeatBannerFrame = 60;
}

void BossCutscene::StartIntro(GameManager& game, const std::shared_ptr<Enemy>& boss)
{
	if (!boss) return;
	m_boss = boss;
	m_isRoarShaken = false;
	m_totalFrame = 0;

	//ショットはプレイヤーのいる側から撮る//背中から撮ると顔が映らない
	Vector3 toPlayer = game.GetPlayer().GetPos() - boss->GetPos();
	toPlayer.y = 0.0f;
	if (toPlayer.SqMagnitude() > 0.0f)
	{
		m_bossToPlayer = toPlayer.Normalize();
	}

	//最後に戻す場所//演出中は普段のカメラを動かさないので、今のポーズがそのまま使える
	m_returnPose = game.GetGameplayCameraPose();

	//プレイヤーとボスを止める//ボスは光の柱から出てきても止まったまま
	game.GetPlayer().SetInputLock(true);
	boss->SetHold(true);

	EventShot shot;
	shot.pose = MakeBossShot(*boss, kWideDistanceRate, kWideHeightRate, kWideLookRate);
	shot.followRate = kWideFollowRate;
	game.StartEventCamera(shot);

	ChangePhase(Phase::IntroAppear);
}

void BossCutscene::Update(GameManager& game)
{
	if (m_phase == Phase::None || m_phase == Phase::DefeatDone) return;

	m_phaseFrame++;
	m_totalFrame++;

	auto boss = m_boss.lock();

	//撃破の演出はボスが消えた後も続く
	if (m_phase == Phase::DefeatSlow || m_phase == Phase::DefeatFall || m_phase == Phase::DefeatBanner)
	{
		UpdateDefeat(game, boss.get());
		return;
	}

	//登場の途中でボスがいなくなったらすぐ終える
	if (!boss)
	{
		FinishIntro(game);
		return;
	}

	//カメラを戻す前なら決定ボタンで飛ばせる
	const bool canSkip = (m_phase == Phase::IntroAppear || m_phase == Phase::IntroRoar) && m_totalFrame >= kSkipStartFrame;
	if (canSkip && Input::Instance().IsTriggered("ok"))
	{
		StartReturn(game);
		return;
	}

	switch (m_phase)
	{
	case Phase::IntroAppear:
		//光の柱から出てきったら叫ばせて顔に寄る
		if (!boss->IsAppearing())
		{
			boss->Roar();
			//カメラの位置は叫んでいる間動かさない//見る点だけ頭を追う
			const float height = boss->GetHeight();
			m_roarCameraPos = boss->GetPos() + m_bossToPlayer * (height * kCloseDistanceRate) + Vector3(0.0f, height * kCloseHeightRate, 0.0f);
			ChangePhase(Phase::IntroRoar);
		}
		break;

	case Phase::IntroRoar:
	{
		//前のめりになって下がっていく頭を追う
		EventShot shot;
		shot.pose = MakeRoarShot(*boss);
		shot.followRate = kCloseFollowRate;
		game.SetEventShot(shot);
	}
		//叫びのピークで画面を揺らす
		if (!m_isRoarShaken && m_phaseFrame >= kRoarShakeFrame)
		{
			m_isRoarShaken = true;
			game.ShakeCamera(kRoarShakePower, kRoarShakeLength);
		}
		//叫び終わったらカメラを戻す
		//Roarを呼んだフレームはまだステートが切り替わっていないので、1フレーム目は見ない
		if ((m_phaseFrame > 1 && !boss->IsRoaring()) || m_phaseFrame >= kRoarMaxFrame)
		{
			StartReturn(game);
		}
		break;

	case Phase::IntroReturn:
		if (game.IsEventShotArrived() || m_phaseFrame >= kReturnMaxFrame)
		{
			FinishIntro(game);
		}
		break;

	default:
		break;
	}
}

void BossCutscene::ChangePhase(Phase phase)
{
	m_phase = phase;
	m_phaseFrame = 0;
}

void BossCutscene::StartReturn(GameManager& game)
{
	EventShot shot;
	shot.pose = m_returnPose;
	shot.followRate = kReturnFollowRate;
	game.SetEventShot(shot);
	ChangePhase(Phase::IntroReturn);
}

void BossCutscene::FinishIntro(GameManager& game)
{
	//最後に映していたところから普段のカメラを作り直す
	game.EndEventCamera();
	game.GetPlayer().SetInputLock(false);
	//飛ばしたときは叫んでいる途中でも動き出す(叫び終わってから追いかけてくる)
	//撃破の演出で使うので、ボスは覚えたままにする
	if (auto boss = m_boss.lock())
	{
		boss->SetHold(false);
	}
	ChangePhase(Phase::None);
}

void BossCutscene::CheckDefeat(GameManager& game)
{
	//登場の途中はボスが無敵なので倒れない//撃破の演出は1回だけ
	if (m_phase != Phase::None) return;
	auto boss = m_boss.lock();
	if (!boss || boss->GetHp() > 0) return;

	//とどめで長く止めて大きく揺らす
	Timer& timer = Timer::Instance();
	timer.RequestHitStop(kDefeatHitStopFrame);
	game.ShakeCamera(kDefeatShakePower, kDefeatShakeLength);
	//ウィッチタイム中だとスローと重なって止まって見えるので、敵だけ遅いのは終わらせる
	timer.SetEnemyTimeScaleForFrames(1.0f, 0);
	//ヒットストップの間はスローの残りが減らないので、止まり終わってからスローになる
	timer.SetTimeScaleForFrames(kDefeatSlowScale, kDefeatSlowFrame);

	//決着がついたら操作は終わり//クリア画面へ切り替わるまで止めたまま
	game.GetPlayer().SetInputLock(true);

	//ショットはプレイヤーとボスを結ぶ線の横から撮る//今のカメラに近い側を選ぶ
	Vector3 toPlayer = game.GetPlayer().GetPos() - boss->GetPos();
	toPlayer.y = 0.0f;
	if (toPlayer.SqMagnitude() > 0.0f)
	{
		m_bossToPlayer = toPlayer.Normalize();
	}
	Vector3 side(m_bossToPlayer.z, 0.0f, -m_bossToPlayer.x);
	const Vector3 toCamera = game.GetGameplayCameraPose().pos - boss->GetPos();
	if (side.Dot(toCamera) < 0.0f)
	{
		side = side * -1.0f;
	}
	const float height = boss->GetHeight();
	m_defeatCameraPos = boss->GetPos() + side * (height * kDefeatDistanceRate) + Vector3(0.0f, height * kDefeatHeightRate, 0.0f);
	m_defeatLookHeight = height * kDefeatLookRate;

	EventShot shot;
	shot.pose = MakeDefeatShot(*boss);
	shot.followRate = kDefeatFollowRate;
	game.StartEventCamera(shot);

	ChangePhase(Phase::DefeatSlow);
}

void BossCutscene::UpdateDefeat(GameManager& game, Enemy* boss)
{
	//倒れて後ろへ飛んでいくボスを目で追う
	if (boss)
	{
		EventShot shot;
		shot.pose = MakeDefeatShot(*boss);
		shot.followRate = kDefeatFollowRate;
		game.SetEventShot(shot);
	}

	switch (m_phase)
	{
	case Phase::DefeatSlow:
		//スローが終わったら普通の速さで倒れきるのを待つ
		if (m_phaseFrame >= kDefeatSlowFrame)
		{
			ChangePhase(Phase::DefeatFall);
		}
		break;

	case Phase::DefeatFall:
		//消えて(GameManagerが片付けて)いなくなったら文字を出す
		if (!boss || boss->IsVanished())
		{
			ChangePhase(Phase::DefeatBanner);
		}
		break;

	case Phase::DefeatBanner:
		if (m_phaseFrame >= kDefeatBannerFrame)
		{
			m_boss.reset();
			//カメラは戻さない//このままクリア画面へ切り替わる
			ChangePhase(Phase::DefeatDone);
		}
		break;

	default:
		break;
	}
}

CameraPose BossCutscene::MakeRoarShot(const Enemy& boss) const
{
	CameraPose pose;
	pose.pos = m_roarCameraPos;
	//体の真ん中と顔の間を見る
	const Vector3 center = boss.GetCenterPos();
	pose.target = center + (boss.GetFacePos() - center) * kCloseFaceRate;
	return pose;
}

CameraPose BossCutscene::MakeDefeatShot(const Enemy& boss) const
{
	CameraPose pose;
	pose.pos = m_defeatCameraPos;
	pose.target = boss.GetPos() + Vector3(0.0f, m_defeatLookHeight, 0.0f);
	return pose;
}

CameraPose BossCutscene::MakeBossShot(const Enemy& boss, float distanceRate, float heightRate, float lookRate) const
{
	const float height = boss.GetHeight();
	const Vector3 base = boss.GetPos();
	CameraPose pose;
	pose.pos = base + m_bossToPlayer * (height * distanceRate) + Vector3(0.0f, height * heightRate, 0.0f);
	pose.target = base + Vector3(0.0f, height * lookRate, 0.0f);
	return pose;
}
