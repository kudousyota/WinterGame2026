#pragma once
#include "../Math/Vector3.h"
class Timer
{
public:
	static Timer& Instance();

	void Update();
	void SetTimeScale(float scale) { m_timeScale = scale; }

	float GetTimeScale() const { return m_timeScale; }

	//敵専用のタイムスケール
	float GetEnemyTimeScale()const { return m_enemyTimeScale; }

	//ゲーム内の時間を取得する
	int GetGameTime()const { return m_gameTime; }

	void SetTimeScaleForFrames(float timescale, int frames);

	void SetEnemyTimeScaleForFrames(float scale, int frames);

	bool IsEnemySlow() const;

	//ヒットストップをリクエストする
	void RequestHitStop(int frame);
	//今ヒットストップ中かどうか
	bool IsHitStop()const { return m_isHitStop; }

	//スロー・ウィッチタイム・ヒットストップを解除する(シーンの切り替え時に呼ぶ)
	//ゲーム内の時間は先行入力の記録とずれないように戻さない
	void Reset();
private:
	//コンストラクタとデストラクタをプライベートにして、シングルトンパターンを実装
	Timer() = default;
	virtual ~Timer() = default;
	//コピーコンストラクタと代入演算子を削除して、シングルトンのインスタンスが複製されないようにする
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	//時間の管理
	float m_timeScale = 1.0f;
	//時間を元に戻すためのやつ
	int m_frameCount = 0;

	float m_enemyTimeScale = 1.0f;
	int m_frameCountEnemy = 0;

	//ゲーム内の時間を図るための変数
	int m_gameTime = 0;

	//ヒットストップの残りフレーム数
	int m_hitStopFrame = 0;
	//このフレームがヒットストップ中かどうか
	bool m_isHitStop = false;
};