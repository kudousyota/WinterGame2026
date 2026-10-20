#pragma once
#include <string>
#include <list>

//フォントの読み込みと、揃え方を指定した文字列の描画をまとめたもの
//使えるサイズ: 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88
class FontManager
{
public:
	static FontManager& Instance();

	//フォントファイルを読み込み、使うサイズのフォントを作っておく
	void LoadFont();
	//作ったフォントとフォントファイルを解放する(DxLib_Endより前に呼ぶ)
	void Destroy();

	//指定座標を中心とする文字列を描画
	void DrawCenteredText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor);
	//指定座標を右下とする文字列を描画
	void DrawBottomRightText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor);
	//指定座標を左端(縦は中央)とする文字列を描画
	void DrawLeftText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor);
	//指定座標を中心とし、拡大した文字列を描画
	void DrawCenteredExtendText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor, float extendRate);
	/// <summary>
	/// 指定座標を右下とする文字列を描画し、フラグがtrueになると文字を揺らす
	/// </summary>
	/// <param name="shakeAmplitude">振れ幅</param>
	/// <param name="shakeSpeed">揺れの速さ</param>
	void DrawBottomRightAndQuakeText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor, bool isShake = false, int shakeAmplitude = 5, int shakeSpeed = 2);

private:
	FontManager() = default;
	virtual ~FontManager() = default;
	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;

	//指定サイズのフォントハンドルを探す(見つからなければ-1)
	int FindHandle(int size)const;

private:
	struct Font
	{
		int size;
		int handle;
	};
	std::list<Font> m_fonts;
};
