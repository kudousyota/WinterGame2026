#pragma once
#include <string>
#include <vector>
#include "Vector3.h"
class SpawnTable
{
public:
	//スポーン位置を何から測るか
	enum class SpawnBase
	{
		//ワールド座標そのまま
		World,
		//ウェーブ開始時のプレイヤーの位置
		Player,
	};

	struct SpawnInfo
	{
		//ウェーブ数
		int wave = 0;
		//キャラクターの名前
		std::string characterName;
		//位置の基準
		SpawnBase base = SpawnBase::World;
		//スポナー位置
		Vector3 spawnerPos = Vector3(0.0f, 0.0f, 0.0f);
		//スポーン位置
		Vector3 offset = Vector3(0.0f, 0.0f, 0.0f);
		//スポーンまでのフレーム数
		int delayFrame = 0;
		//基準からの位置
		Vector3 GetLocalPos() const { return spawnerPos + offset; }
	};

	//戦闘中だけ閉じる見えない壁
	struct GateInfo
	{
		//壁の底の中心
		Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
		//Y軸まわりの向き
		float rotY = 0.0f;
	};

	//ウェーブの戦闘エリア
	//入ったらウェーブが始まって全滅させるまで外に出られない
	struct WaveArea
	{
		Vector3 center = Vector3(0.0f, 0.0f, 0.0f);
		//0ならエリアなし
		float radius = 0.0f;
		//戦闘中に閉じるゲート
		std::vector<GateInfo> gates;

		bool HasArea() const { return radius > 0.0f; }
		//XZだけで中にいるか判定する
		bool IsInside(const Vector3& pos) const;
		//XZを円の内側(半径-margin)に収めた位置を返す(高さはそのまま)
		Vector3 ClampInside(const Vector3& pos, float margin) const;
		bool IsSame(const WaveArea& other) const;
	};

	//CSVの行データを受け取る(spawnRows:Spawn.csv waveRows:Wave.csv gateRows:Gate.csv)
	void Load(const std::vector<std::vector<std::string>>& spawnRows,
	          const std::vector<std::vector<std::string>>& waveRows,
	          const std::vector<std::vector<std::string>>& gateRows);

	//そのウェーブで出る敵の一覧
	const std::vector<SpawnInfo>& GetWave(int wave)const;
	//そのウェーブの戦闘エリア
	const WaveArea& GetArea(int wave)const;
	//ウェーブ数を取得する
	int GetWaveCount()const { return static_cast<int>(m_waves.size()); }
	//1つのウェーブで同時に閉じるゲートの最大数(ゲートのモデルを何個用意するかに使う)
	int GetMaxGateNum()const;

private:
	//ウェーブ番号そのウェーブの何体目か
	std::vector<std::vector<SpawnInfo>> m_waves;
	//ウェーブ番号ごとの戦闘エリア
	std::vector<WaveArea> m_areas;
};
