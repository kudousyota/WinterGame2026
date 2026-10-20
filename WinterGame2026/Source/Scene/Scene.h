#pragma once
class SceneController;

//シーンの基底クラス
class Scene
{
public:
	Scene(SceneController& controller);
	virtual ~Scene() = default;

	//シーンが積まれた(切り替わった)ときに一度だけ呼ばれる
	virtual void Init() = 0;
	//シーンの更新(一番上に積まれているシーンだけ呼ばれる)
	//入力はInput::Instance()から取る
	virtual void Update() = 0;
	//シーンの描画(積まれている全シーンが下から順に呼ばれる)
	virtual void Draw() = 0;

protected:
	//シーンの切り替えを頼むため(所有しない)
	SceneController& m_controller;
};
