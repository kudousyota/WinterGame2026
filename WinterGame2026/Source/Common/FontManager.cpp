#include "FontManager.h"
#include "DxLib.h"
#include <cassert>
#include <cmath>

namespace
{
	const char* const kFontPath = "Data/Font/SourceHanSerifJP-ExtraLight.otf";
	const char* const kFontName = "源ノ明朝 JP ExtraLight";

	//最初に作っておくフォントのサイズ
	const int kFontSize[] =
	{
		8,16,24,32,40,48,56,64,72,80,88
	};
}

FontManager& FontManager::Instance()
{
	static FontManager instance;
	return instance;
}

void FontManager::LoadFont()
{
	//フォントファイルをこのアプリの中だけで使えるようにする
	if (AddFontResourceEx(kFontPath, FR_PRIVATE, NULL) <= 0)
	{
		//フォント読込エラー処理
		MessageBox(NULL, "フォント読込失敗", "", MB_OK);
	}

	//とりあえず使いそうなサイズのフォントを作っておく
	for (auto& size : kFontSize)
	{
		Font add;
		add.size = size;
		add.handle = CreateFontToHandle(kFontName, size, -1, DX_FONTTYPE_EDGE, -1, 1);
		m_fonts.push_back(add);
	}
}

void FontManager::Destroy()
{
	for (const auto& f : m_fonts)
	{
		DeleteFontToHandle(f.handle);
	}
	m_fonts.clear();
	RemoveFontResourceEx(kFontPath, FR_PRIVATE, NULL);
}

int FontManager::FindHandle(int size) const
{
	for (auto& font : m_fonts)
	{
		if (font.size == size)
		{
			return font.handle;
		}
	}
#ifdef _DEBUG
	assert(0 && "フォントデータが見つかりませんでした");
#endif
	return -1;
}

void FontManager::DrawCenteredText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor)
{
	int handle = FindHandle(size);
	if (handle == -1) return;

	int textWidth = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle);
	int textHeight = GetFontSizeToHandle(handle);
	DrawStringToHandle(x - textWidth / 2, y - textHeight / 2, text.c_str(), color, handle, edgeColor);
}

void FontManager::DrawBottomRightText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor)
{
	int handle = FindHandle(size);
	if (handle == -1) return;

	int textWidth = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle);
	int textHeight = GetFontSizeToHandle(handle);
	DrawStringToHandle(x - textWidth, y - textHeight, text.c_str(), color, handle, edgeColor);
}

void FontManager::DrawLeftText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor)
{
	int handle = FindHandle(size);
	if (handle == -1) return;

	int textHeight = GetFontSizeToHandle(handle);
	//左端揃えの文字列描画
	DrawStringToHandle(x, y - textHeight / 2, text.c_str(), color, handle, edgeColor);
}

void FontManager::DrawCenteredExtendText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor, float extendRate)
{
	int handle = FindHandle(size);
	if (handle == -1) return;

	int textWidth = static_cast<int>(GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle) * extendRate);
	int textHeight = static_cast<int>(GetFontSizeToHandle(handle) * extendRate);
	DrawExtendStringToHandle(x - textWidth / 2, y - textHeight / 2, extendRate, extendRate, text.c_str(), color, handle, edgeColor);
}

void FontManager::DrawBottomRightAndQuakeText(int x, int y, const std::string& text, unsigned int color, int size, unsigned int edgeColor, bool isShake, int shakeAmplitude, int shakeSpeed)
{
	int handle = FindHandle(size);
	if (handle == -1) return;

	if (isShake)
	{
		//現在のフレーム数を取得
		int frameCount = GetNowCount() / shakeSpeed / 5;
		//揺れの計算(sinを使用して滑らかに左右に動く)
		int shakeOffset = static_cast<int>(sin(frameCount) * shakeAmplitude / 6);

		x += shakeOffset;
	}

	int textWidth = GetDrawStringWidthToHandle(text.c_str(), static_cast<int>(text.length()), handle);
	int textHeight = GetFontSizeToHandle(handle);
	DrawStringToHandle(x - textWidth, y - textHeight, text.c_str(), color, handle, edgeColor);
}
