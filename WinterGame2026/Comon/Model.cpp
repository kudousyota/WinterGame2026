#include "Model.h"
#include <DxLib.h>
//ベースモデルを一括でロードする
//起動時に一度だけ呼び出し、作成して利用する
void Model::PreloadAll()
{
	m_playerBaseHandle = MV1LoadModel("Data/Model/Player.mv1");
}
//ロードしたベースモデルを解放する
//ゲーム終了時に呼び出す
void Model::ReleseAll()
{
	if(m_playerBaseHandle != -1)MV1DeleteModel(m_playerBaseHandle);
}
