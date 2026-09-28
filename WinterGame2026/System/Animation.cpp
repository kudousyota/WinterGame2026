#include "Animation.h"
#include <DxLib.h>
#include "Timer.h"
namespace
{
	//アニメーションを切り替えるフレーム数
	constexpr int kAnimChangeFrame = 10;
}
Animation::Animation()
{
}

Animation::~Animation()
{
	//アタッチしているアニメーションが残るとリソースリークなどの原因になるのでデタッチする。
	//モデル自体はもっていないため削除しない
	if (m_modelHandle != -1)
	{
		if (m_currentAnimHandle != -1)
		{
			MV1DetachAnim(m_modelHandle, m_currentAnimHandle);
			m_currentAnimHandle = -1;
		}
		if (m_prevAnimHandle != -1)
		{
			MV1DetachAnim(m_modelHandle, m_prevAnimHandle);
			m_prevAnimHandle = -1;
		}
	}
	
}

void Animation::Init(int modelHandle, std::string name, bool isRoop, float timeScale)
{
	SetAnim(isRoop);
	//モデルのハンドルを保存
	m_modelHandle = modelHandle;
	//animmation名からアニメションのインデックスを取得
	int animNo = MV1GetAnimIndex(modelHandle, name.c_str());
	//m_animHandleに入れることで,アニメーションを変えるときや、ブレンドするときにm_animHandleを使う
	//アニメーションをアタッチする
	m_currentAnimHandle = MV1AttachAnim(m_modelHandle, animNo, -1, -1);
	//アニメションのフレーム数0を0にする
	m_currentAnimCount = 0.0f;
	//前のアニメションのハンドルを-1にする
	m_prevAnimHandle = -1;
	//前のアニメションのフレーム数を0にする
	m_animTimeScale = timeScale;

	m_currentAnimName = name;
}

void Animation::Update(float speed)
{
	//時間のスケールを取得//0から1の値を返す
	float timeScale = Timer::Instance().GetTimeScale();
	//アニメーションの更新
	//アニメーションのフレーム更新
	m_currentAnimCount += 1.0f * timeScale * m_animTimeScale * speed;
	//前のアニメーションの更新
	m_prevAnimCount += 1.0f * timeScale * m_prevAnimTimeScale * speed;
	//アニメーションのブレンド
	AnimBlend();
	//アニメーションのループ処理
	//アタッチしたアニメーションの総フレームを取得
	float totalAnimCount = MV1GetAttachAnimTotalTime(m_modelHandle, m_currentAnimHandle);
	if (m_currentAnimCount >= totalAnimCount)
	{
		m_isEndAnim = true;//アニメーションが終わったフラグを立てる
		if (m_isRoop)
		{
			m_currentAnimCount -= totalAnimCount;//アニメーションのフレーム数がアニメーションの総フレーム数を超えたら、0にする
		}
		else
		{
			m_currentAnimCount = totalAnimCount;//止める
		}
	}
	//アニメーションのカウントを更新する
	MV1SetAttachAnimTime(m_modelHandle, m_currentAnimHandle, m_currentAnimCount);
	//前のアニメーションのループ再生
	if (m_prevAnimHandle != -1)
	{
		float totalPrevAnimCount = MV1GetAttachAnimTotalTime(m_modelHandle, m_prevAnimHandle);
		/*printfDx(
			"Handle:%d Time:%f Total:%f\n",
			m_currentAnimHandle,
			m_currentAnimCount,
			totalAnimCount);*/

		if (m_prevAnimCount >= totalPrevAnimCount)
		{
			if (m_prevRoop)
			{
				m_prevAnimCount -= totalPrevAnimCount;//前のアニメーションのフレーム数が前のアニメーションの総フレーム数を超えたら、0にする
			}
			else
			{
				m_prevAnimCount = totalPrevAnimCount;//止める
			}
		}

		MV1SetAttachAnimTime(m_modelHandle, m_prevAnimHandle, m_prevAnimCount);//前のアニメーションのフレーム数を更新する
	}
}

void Animation::AnimBlend()
{
	if (m_prevAnimHandle == -1)
	{
		//アニメーションが一つしかないときは、ブレンドしない
		MV1SetAttachAnimBlendRate(m_modelHandle, m_currentAnimHandle, 1.0f);//現在のアニメーションのブレンド率を1にする
	}
	else
	{
		//timeScale
		float timeScale = Timer::Instance().GetTimeScale();//時間のスケールを取得する//0から1の値を返す//0.5なら、時間が半分になる

		m_animChangeFrame += 1.0f * timeScale;//アニメーションを切り替えるフレーム数を増やす
		//currentAnimBlendのブレンド率を計算
		float rate = m_animChangeFrame / kAnimChangeFrame;//アニメーションを切り替えるフレーム数で割ることで、0から1までの値を作る
		if (rate > 1.0f)
		{
			rate = 1.0f;//ブレンド率が1を超えないようにする
			//ブレンド完了
			MV1DetachAnim(m_modelHandle, m_prevAnimHandle);
			m_prevAnimHandle = -1;
			m_prevAnimCount = 0.0f;
		}
		MV1SetAttachAnimBlendRate(m_modelHandle, m_currentAnimHandle, rate);//現在のアニメーションのブレンド率を設定する
		if (m_prevAnimHandle != -1)//prevが有効な時だけ呼ぶ//prevが-1の時にSetAttachAnimBlendRateを呼ぶと、そのハンドルのブレンド率が0になってしまうので、呼ばないようにする
		{
			MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimHandle, 1.0f - rate);//前のアニメーションのブレンド率を設定する
		}
	}
}

void Animation::SetAnim(bool isRoop)
{
	m_prevRoop = m_isRoop;
	//ループするかどうかの保存
	m_isRoop = isRoop;
	//アニメーションの終了フラグを初期化
	m_isEndAnim = false;
}

void Animation::ChangeAnim(std::string name, bool isRoop, float timeScale)
{
	m_prevAnimTimeScale = m_animTimeScale;//前のアニメーションの再生速度を保存する
	m_animTimeScale = timeScale;//アニメーションの再生速度を設定する

	//アニメーションの切り替え
	//攻撃終了の旗、ループするかどうかの旗を初期化
	SetAnim(isRoop);
	//新しくアニメーションをセットしようとしたとき
	//古いアニメーションが残っている場合、それをデタッチ
	if (m_prevAnimHandle != -1)
	{
		MV1DetachAnim(m_modelHandle, m_prevAnimHandle);//前のアニメーションをデタッチする
		m_prevAnimHandle = -1;//前のアニメーションのハンドルを-1にする
		m_prevAnimCount = 0.0f;//前のアニメーションのフレーム数を0にする
	}
	//現在再生中のアニメーションを一つ前のアニメーションとする
	m_prevAnimHandle = m_currentAnimHandle;//現在のアニメーションを保存する
	m_prevAnimCount = m_currentAnimCount;//現在のアニメーションのフレーム数を保存する
	//デタッチしたら消えてしまうので、デタッチせず、新しいアニメーションをアタッチする
	int animNo = MV1GetAnimIndex(m_modelHandle, name.c_str());//アニメーション名からアニメーションのインデックスを取得する
	m_currentAnimHandle = MV1AttachAnim(m_modelHandle, animNo, -1, -1);//新しいアニメーションをアタッチする
	MV1SetAttachAnimBlendRate(m_modelHandle, m_currentAnimHandle, 0.0f); // 新アニメを非表示から開始
	MV1SetAttachAnimBlendRate(m_modelHandle, m_prevAnimHandle, 1.0f);    // 旧アニメを完全表示に明示
	m_currentAnimCount = 0.0f;//新しいアニメーションのフレーム数を0にする
	m_animChangeFrame = 0.0f;//アニメーションを切り替えるフレーム数を0にする
	m_currentAnimName = name;
}

float Animation::GetAnimRate()
{
	float totalAnimCount = MV1GetAttachAnimTotalTime(m_modelHandle, m_currentAnimHandle);//アタッチしているアニメーションの総フレーム数を取得する
	if (totalAnimCount <= 0.0f)	return 0.0f;//総フレーム数が0以下のときは、アニメーションの進行率を0にする//0で割るのを防ぐ

	//アニメーションの進行割合
	float rate = m_currentAnimCount / totalAnimCount;//アニメーションのフレーム数をアニメーションの総フレーム数で割ることで、0から1までの値を作る
	return rate;
}
