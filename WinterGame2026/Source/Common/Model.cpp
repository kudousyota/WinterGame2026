#include "Model.h"
#include <DxLib.h>
//ベースモデルを一括でロードする
//起動時に一度だけ呼び出して作成して利用する
void Model::PreloadAll()
{
	m_playerBaseHandle = MV1LoadModel("Data/Model/Player.mv1");
	m_alienBaseHandle = MV1LoadModel("Data/Model/Alien.mv1");
	m_warrokBaseHandle = MV1LoadModel("Data/Model/Warrok.mv1");
	m_goblinBaseHandle = MV1LoadModel("Data/Model/Goblin.mv1");
	m_gateBaseHandle = MV1LoadModel("Data/Model/Gate.mv1");
}
//ロードしたベースモデルを解放する
//ゲーム終了時に呼び出す
void Model::ReleseAll()
{
	if (m_playerBaseHandle != -1)MV1DeleteModel(m_playerBaseHandle);
	if (m_alienBaseHandle != -1)MV1DeleteModel(m_alienBaseHandle);
	if (m_warrokBaseHandle != -1)MV1DeleteModel(m_warrokBaseHandle);
	if (m_goblinBaseHandle != -1)MV1DeleteModel(m_goblinBaseHandle);
	if (m_gateBaseHandle != -1)MV1DeleteModel(m_gateBaseHandle);
}
