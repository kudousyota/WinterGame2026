#pragma once
#include "CameraPose.h"

//演出用のカメラが向かう先
struct EventShot
{
	//カメラの位置と見る点
	CameraPose pose;
	//1フレームで残りの何割寄るか(0～1)//1でパッと切り替わる
	float followRate = 0.1f;
};

//ボスの登場や撃破などの演出で使うカメラ
//「どう映すか」だけを持つ。「いつ何を映すか」は演出の係のクラスが決める
//スローやヒットストップの影響は受けない(演出は普通の速さで見せる)
class EventCamera
{
public:
	//演出を始める
	//from:今のカメラのポーズ//ここから寄せるので、始まった瞬間に画面が飛ばない
	void Start(const CameraPose& from, const EventShot& shot);
	//演出の途中で次のショットへ切り替える//今の位置から新しいショットへ寄せる
	void SetShot(const EventShot& shot);
	//今のショットへ1フレーム分寄せる
	void Update();
	//演出を終える//普段のカメラへのつなぎはCamera::SyncFromPoseで行う
	void End();

	bool IsActive()const { return m_isActive; }
	//今のポーズ
	const CameraPose& GetPose()const { return m_pose; }
	//今のショットにほぼ着いたか//次のショットへ進むタイミングに使う
	bool IsArrived()const;

private:
	bool m_isActive = false;
	//今のポーズ
	CameraPose m_pose;
	//向かっているショット
	EventShot m_shot;
};
