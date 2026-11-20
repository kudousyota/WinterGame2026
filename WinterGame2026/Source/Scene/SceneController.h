#pragma once
#include <memory>
#include <list>

class Scene;

//シーンをスタックで持ち切り替えを行う
class SceneController
{
public:
	//積まれているシーンを全部消して渡したシーンだけにする
	void ResetScene(std::shared_ptr<Scene> scene);
	//一番上のシーンを渡したシーンに入れ替える
	void ChangeScene(std::shared_ptr<Scene> scene);
	//今のシーンを残したまま、上に新しいシーンを積む
	void PushScene(std::shared_ptr<Scene> scene);
	//一番上のシーンを外す
	void PopScene();

	//予約された切り替えとフェードを進めてから一番上のシーンを更新する
	//フェード中はシーンを更新しない
	void Update();
	//積まれている全シーンを下から順に描画しフェード中なら黒を重ねる
	void Draw();

private:
	//予約されている切り替えを行う
	void ApplyRequest();

private:
	//切り替えの種類
	enum class RequestType
	{
		None,
		Reset,
		Change,
		Push,
		Pop,
	};

	//フェードの状態
	enum class FadeState
	{
		None,
		//暗くしている途中
		Out,
		//明るくしている途中
		In,
	};

	//積まれているシーン
	std::list<std::shared_ptr<Scene>> m_scenes;

	//予約されている切り替え
	RequestType m_requestType = RequestType::None;
	std::shared_ptr<Scene> m_requestScene;

	FadeState m_fadeState = FadeState::None;
	//フェードの進み具合
	int m_fadeFrame = 0;
};
