#include "EventCamera.h"

namespace
{
	//目標との差がこれより小さくなったら着いたとみなす
	constexpr float kArriveDistance = 1.0f;
}

void EventCamera::Start(const CameraPose& from, const EventShot& shot)
{
	m_isActive = true;
	m_pose = from;
	m_shot = shot;
}

void EventCamera::SetShot(const EventShot& shot)
{
	m_shot = shot;
}

void EventCamera::Update()
{
	if (!m_isActive) return;

	//位置と見る点を別々に寄せる
	//見る点も補間するので、向きが急に変わらない
	m_pose.pos += (m_shot.pose.pos - m_pose.pos) * m_shot.followRate;
	m_pose.target += (m_shot.pose.target - m_pose.target) * m_shot.followRate;
}

void EventCamera::End()
{
	m_isActive = false;
}

bool EventCamera::IsArrived() const
{
	const float arrive2 = kArriveDistance * kArriveDistance;
	return (m_shot.pose.pos - m_pose.pos).SqMagnitude() < arrive2
		&& (m_shot.pose.target - m_pose.target).SqMagnitude() < arrive2;
}
