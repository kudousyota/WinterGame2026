#pragma once
#include <memory>
#include "CameraPose.h"

class GameManager;
class Enemy;

//ボスの登場と撃破の演出の進行役
//「いつ何を映すか」と、その間プレイヤーとボスを止めることだけを持つ
//「どう映すか」はEventCamera、文字や黒帯はUIが受け持つ
class BossCutscene
{
public:
	//演出のどこにいるか
	enum class Phase
	{
		//演出していない
		None,
		//登場：ボスが光の柱から出てくるのを引きで映す
		IntroAppear,
		//登場：顔に寄って叫ぶ
		IntroRoar,
		//登場：カメラをプレイヤーの後ろへ戻す
		IntroReturn,
		//撃破：とどめのヒットストップのあと、スローで倒れていくのを横から映す
		DefeatSlow,
		//撃破：普通の速さに戻って、倒れて消えるまで映す
		DefeatFall,
		//撃破：消えたあと「BOSS DEFEATED」を見せる
		DefeatBanner,
		//撃破：演出は終わり//シーンがクリア画面へ切り替えるまで文字を出したままにする
		DefeatDone,
	};

	//登場の演出を始める//ボスを出した直後(StartAppearの後)に呼ぶ
	void StartIntro(GameManager& game, const std::shared_ptr<Enemy>& boss);
	//1フレーム進める//ヒットストップ中は呼ばない
	void Update(GameManager& game);
	//ボスのHPが0になっていたら撃破の演出を始める
	//とどめのヒットストップに間に合うよう、攻撃判定の直後(ヒットストップが始まる前)に呼ぶ
	void CheckDefeat(GameManager& game);

	//演出中か//演出中はプレイヤーを操作できず、シーンは勝ち負けの切り替えを待つ
	//撃破の演出が終わった後(DefeatDone)は待たせない
	bool IsPlaying()const { return m_phase != Phase::None && m_phase != Phase::DefeatDone; }
	//UI用//今どこにいて、そこに入ってから何フレーム経ったか
	Phase GetPhase()const { return m_phase; }
	int GetPhaseFrame()const { return m_phaseFrame; }
	//UI用//演出しているボス//登場の演出を始めてから撃破の演出が終わるまで持っている//いなければ空
	std::shared_ptr<const Enemy> GetBoss()const { return m_boss.lock(); }

private:
	void ChangePhase(Phase phase);
	//カメラを戻し始める//飛ばしたときもここへ来る
	void StartReturn(GameManager& game);
	//登場の演出を終えてプレイヤーとボスを動かす//撃破の演出のためにボスは覚えたままにする
	void FinishIntro(GameManager& game);
	//撃破の演出を進める//ボスが消えた後も進むので、ボスはいなくてもよい
	void UpdateDefeat(GameManager& game, Enemy* boss);
	//叫ぶボスを足元近くから見上げるショット//カメラは止めたまま、見る点だけ頭を追う
	CameraPose MakeRoarShot(const Enemy& boss)const;
	//倒れていくボスを横から映すショット//カメラは止めたまま、見る点だけボスについていく
	CameraPose MakeDefeatShot(const Enemy& boss)const;

	//ボスの正面からのショットを作る
	//distanceRate:ボスからカメラまでの距離(身長に対する割合)
	//heightRate:カメラの高さ、lookRate:見る点の高さ(どちらも身長に対する割合)
	CameraPose MakeBossShot(const Enemy& boss, float distanceRate, float heightRate, float lookRate)const;

	Phase m_phase = Phase::None;
	int m_phaseFrame = 0;
	//演出が始まってからのフレーム数
	int m_totalFrame = 0;
	//演出しているボス//倒されたり消されたりしても触らないようにweak_ptrで持つ
	std::weak_ptr<Enemy> m_boss;
	//ボスから見たプレイヤーの方向(水平)//ショットはこちら側から撮る
	Vector3 m_bossToPlayer = Vector3(0.0f, 0.0f, -1.0f);
	//演出の前に普段のカメラが映していたところ//最後にここへ戻す
	CameraPose m_returnPose;
	//叫びの揺れをもう出したか
	bool m_isRoarShaken = false;
	//叫ぶショットのカメラの位置//叫んでいる間は動かさない
	Vector3 m_roarCameraPos = Vector3(0.0f, 0.0f, 0.0f);
	//撃破のショットのカメラの位置//倒れていく間は動かさない
	Vector3 m_defeatCameraPos = Vector3(0.0f, 0.0f, 0.0f);
	//撃破のショットの見る点の高さ//倒れるときにボスの位置が動いても同じ高さを見る
	float m_defeatLookHeight = 0.0f;
};
