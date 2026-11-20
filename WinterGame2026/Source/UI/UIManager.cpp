#include "UIManager.h"
#include "DxLib.h"

void UIManager::Add(std::unique_ptr<UIBase> ui)
{
	//UIを登録する
	//unique_ptrで所有権をもらい呼び出し側はstd::move(ui)で渡す
	m_pUiList.push_back(std::move(ui));
}

void UIManager::Init()
{
	//登録されたUIのInitをまとめて呼ぶ
	for (auto& ui : m_pUiList)
	{
		ui->Init();
	}

}
void UIManager::Update(Input& input)
{
	//登録されたUIのUpdateをまとめて呼ぶ
	for (auto& ui : m_pUiList)
	{
		ui->Update(input);
	}
}
void UIManager::Draw()
{
	//登録されたUIのDrawをまとめて呼ぶ
	for (auto& ui : m_pUiList)
	{
		ui->Draw();
	}
}
