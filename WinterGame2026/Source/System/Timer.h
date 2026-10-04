#pragma once
#include "../Math/Vector3.h"
class Timer
{
public:
	static Timer& Instance();
	void SetTimeScale(float scale) { m_timeScale = scale; }

	float GetTimeScale() const { return m_timeScale; }

	//敵専用のタイムスケール
	float GetEnemyTimeScale()const { return m_enemyTimeScale; }

	void Update();

	void SetTimeScaleForFrames(float timescale, int frames);

	void SetEnemyTimeScaleForFrames(float scale, int frames);

	bool IsEnemySlow() const;

	////ヒットストップをリクエストする
	//void RequestHitStop(int frame);
	////今ヒットストップ中かどうか
	//bool IsHitStop()const { return m_hitStopFrame > 0; }
	////カメラシェイクをリクエスト
	//void RequestShake(float power, int frame);
	////今フレームのシェイクオフセットを取得する
	//Vector3 GetShakeOffset() const;
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

	int m_hitStopFrame = 0;
};