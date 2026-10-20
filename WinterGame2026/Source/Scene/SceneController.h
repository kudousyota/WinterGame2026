#pragma once
#include <memory>
#include <list>

class Scene;

//シーンをスタックで持ち、切り替えを行う
//Change/Push/Pop/Resetは予約だけして、実際の切り替えは次のUpdateで行う
//(シーンのUpdate中に自分自身を破棄しないようにするため)
//Change/Resetはフェードアウト→切り替え→フェードインを自動で行う
class SceneController
{
public:
	//積まれているシーンを全部消して、渡したシーンだけにする(フェードあり)
	void ResetScene(std::shared_ptr<Scene> scene);
	//一番上のシーンを渡したシーンに入れ替える(フェードあり)
	void ChangeScene(std::shared_ptr<Scene> scene);
	//今のシーンを残したまま、上に新しいシーンを積む(ポーズ画面など。フェードなし)
	void PushScene(std::shared_ptr<Scene> scene);
	//一番上のシーンを外す(フェードなし)
	void PopScene();

	//予約された切り替えとフェードを進めてから、一番上のシーンを更新する
	//フェード中はシーンを更新しない
	void Update();
	//積まれている全シーンを下から順に描画し、フェード中なら黒を重ねる
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
		//暗くしている途中(終わったらシーンを切り替える)
		Out,
		//明るくしている途中
		In,
	};

	//積まれているシーン(最後に積んだものが一番上)
	std::list<std::shared_ptr<Scene>> m_scenes;

	//予約されている切り替え(1フレームに複数頼まれたら最後のものを使う)
	RequestType m_requestType = RequestType::None;
	std::shared_ptr<Scene> m_requestScene;

	FadeState m_fadeState = FadeState::None;
	//フェードの進み具合(0:真っ暗でない 〜 kFadeFrame:真っ暗)
	int m_fadeFrame = 0;
};
