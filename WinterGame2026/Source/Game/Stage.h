#pragma once

class Stage
{
public:
	//どのステージを読み込むか
	enum class Type
	{
		//本編の神殿
		Temple,
		//チュートリアルの部屋
		Tutorial,
	};

	Stage();
	~Stage();

	void Init(Type type);
	void Draw();

	//当たり判定に使うモデルのハンドル
	int GetCollisionHandle()const { return m_collHandle; }

private:
	//空を描く
	void DrawSky() const;

	//描画に使うモデル
	int m_modelHandle = -1;
	//当たり判定に使うモデル
	//神殿はポリゴンが多くて判定が重いから箱や円柱で作った軽いモデルを別に用意している
	int m_collHandle = -1;
	//遠景のモデル
	int m_bgHandle = -1;
};
