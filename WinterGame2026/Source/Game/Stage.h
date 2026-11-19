#pragma once

//ステージ
//ステージのモデルを読み込み、描画と当たり判定用のハンドルを提供する
class Stage
{
public:
	//どのステージを読み込むか
	enum class Type
	{
		//本編の神殿(広場→通路→神殿→ボス広場)
		Temple,
		//チュートリアルの部屋(神殿の大広間から柱を抜いたもの。外には出られない)
		Tutorial,
	};

	Stage();
	~Stage();

	void Init(Type type);
	void Draw();

	//当たり判定に使うモデルのハンドル
	int GetCollisionHandle()const { return m_collHandle; }

private:
	//空(画面全体のグラデーション)を描く
	void DrawSky() const;

	//描画に使うモデル(地面・神殿)
	int m_modelHandle = -1;
	//当たり判定に使うモデル(描画しない)
	//神殿はポリゴンが多くて判定が重いので、箱や円柱で作った軽いモデルを別に用意している
	int m_collHandle = -1;
	//遠景のモデル(描画だけ・当たり判定なし)
	int m_bgHandle = -1;
};
