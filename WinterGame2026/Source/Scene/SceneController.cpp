#include "SceneController.h"
#include "Scene.h"
#include "DxLib.h"
#include "../Game.h"

namespace
{
	//フェードアウト・フェードインそれぞれにかけるフレーム数
	constexpr int kFadeFrame = 60;
}

void SceneController::ResetScene(std::shared_ptr<Scene> scene)
{
	m_requestType = RequestType::Reset;
	m_requestScene = scene;
}

void SceneController::ChangeScene(std::shared_ptr<Scene> scene)
{
	m_requestType = RequestType::Change;
	m_requestScene = scene;
}

void SceneController::PushScene(std::shared_ptr<Scene> scene)
{
	m_requestType = RequestType::Push;
	m_requestScene = scene;
}

void SceneController::PopScene()
{
	m_requestType = RequestType::Pop;
	m_requestScene = nullptr;
}

void SceneController::Update()
{
	switch (m_fadeState)
	{
	case FadeState::Out:
		//真っ暗になったら、見えていないうちにシーンを入れ替えて明るくしていく
		m_fadeFrame++;
		if (m_fadeFrame >= kFadeFrame)
		{
			ApplyRequest();
			m_fadeState = FadeState::In;
		}
		return;
	case FadeState::In:
		m_fadeFrame--;
		if (m_fadeFrame <= 0)
		{
			m_fadeFrame = 0;
			m_fadeState = FadeState::None;
		}
		return;
	case FadeState::None:
		break;
	}

	//前のフレームで頼まれた切り替えを、どのシーンも動いていない今のうちに行う
	if (m_requestType == RequestType::Change || m_requestType == RequestType::Reset)
	{
		if (m_scenes.empty())
		{
			//最初のシーンは暗くするものがないので、真っ暗から明るくするだけ
			ApplyRequest();
			m_fadeFrame = kFadeFrame;
			m_fadeState = FadeState::In;
		}
		else
		{
			m_fadeFrame = 0;
			m_fadeState = FadeState::Out;
		}
		return;
	}
	//Push/Popはフェードせずにすぐ切り替える
	ApplyRequest();

	if (m_scenes.empty()) return;
	//一番上のシーンだけ更新する
	m_scenes.back()->Update();
}

void SceneController::Draw()
{
	//下に積まれているシーンも描画する(ポーズ画面の後ろにゲーム画面を出すため)
	for (auto& scene : m_scenes)
	{
		scene->Draw();
	}

	//フェード中は画面全体に黒を重ねる
	if (m_fadeState != FadeState::None)
	{
		//0.0(重ねない)〜1.0(真っ暗)
		float rate = static_cast<float>(m_fadeFrame) / static_cast<float>(kFadeFrame);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * rate));
		DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, GetColor(0, 0, 0), true);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void SceneController::ApplyRequest()
{
	switch (m_requestType)
	{
	case RequestType::None:
		return;
	case RequestType::Reset:
		m_scenes.clear();
		m_scenes.push_back(m_requestScene);
		break;
	case RequestType::Change:
		//空のときにback()を使うとクラッシュするので、積むだけにする
		if (!m_scenes.empty())
		{
			m_scenes.pop_back();
		}
		m_scenes.push_back(m_requestScene);
		break;
	case RequestType::Push:
		m_scenes.push_back(m_requestScene);
		break;
	case RequestType::Pop:
		if (!m_scenes.empty())
		{
			m_scenes.pop_back();
		}
		break;
	}

	//新しく積んだシーンはここで初期化する(Popのときは何も積んでいない)
	if (m_requestScene)
	{
		m_scenes.back()->Init();
	}

	m_requestType = RequestType::None;
	m_requestScene = nullptr;
}
