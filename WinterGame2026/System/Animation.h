#pragma once
#include <string>
//アニメーション
class Animation
{
public:
	Animation();
	~Animation();
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="modelHandle">モデルのハンドル</param>
	/// <param name="name">アニメーションの名前</param>
	/// <param name="isRoop">ループするかどうか</param>
	/// <param name="timeScale">タイムスケール</param>
	void Init(int modelHandle, std::string name, bool isRoop, float timeScale = 1.0f);
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="speed">アニメーションスピード</param>
	void Update(float speed);
	//アニメーションのブレンド
	void AnimBlend();
	/// <summary>
	/// アニメーションの再生をする
	/// m_isEndを初期化
	/// </summary>
	/// <param name="isRoop">ループ再生するかどうか</param>
	void SetAnim(bool isRoop);
	/// <summary>
	/// アニメーションの切り替え
	/// </summary>
	/// <param name="name">アニメーションの名前</param>
	/// <param name="isRoop">ループ再生するかどうか</param>
	/// <param name="timeScale">再生速度</param>
	void ChangeAnim(std::string name, bool isRoop = true, float timeScale = 1.0f);
	//アニメーションが終わったかどうか
	bool GetAnimEndFlag() { return m_isEndAnim; }
	//アニメションの進行率を返す//0から１の値を返す
	float GetAnimRate();
	//現在のアニメーションのフレーム数を返す
	float GetCurrentAnimTime() const { return m_currentAnimCount; }
	//現在のアニメーションの名前
	const std::string& GetCurrentAnimName()const { return m_currentAnimName; }
private:
	//モデルのハンドル
	int m_modelHandle = -1;
	//現在のアニメションハンドル
	int m_currentAnimHandle = -1;
	//前のアニメションのハンドル
	int m_prevAnimHandle = -1;

	//現在のアニメションのフレーム数
	float m_currentAnimCount = 0.0f;
	//現在のアニメーション名前
	std::string m_currentAnimName;
	//前のアニメションのフレーム数
	float m_prevAnimCount = 0.0f;
	//アニメションを切り替えるフレーム数
	float m_animChangeFrame = 0.0f;
	//アニメションをループさせるかどうかのフラグ
	bool m_isRoop = false;
	//前のアニメションがループしているかどうかのフラグ
	bool m_prevRoop = false;
	//アニメーションがループしているかどうかのフラグ
	bool m_isEndAnim = false;
	//アニメーションの再生速度を管理するための変数
	float m_animTimeScale = 1.0f;
	//前のアニメーションの再生速度を管理するための変数
	float m_prevAnimTimeScale = 0.0f;
};

