#pragma once
#include "../Math/Vector3.h"
#include <vector>
#include <array>
#include <memory>

enum class EffectType
{
	Hit,
	WickedWeave,
	AttackTelegraph,
	Max
};
class Effect;
class EffectManager
{
public:
	static EffectManager& Instns();
	void Init();
	void Update();
	void Draw();
	//再生中のエフェクト停止とロード済みリソースの解放
	void Terminate();
	//指定した種類と座標でエフェクトを再生する
	//rotは各軸の回転(ラジアン。xが上下の傾き、yが水平の向き)
	//scaleは拡大率(1単位=1cmなのでEffekseer側の大きさに合わせて拡大する)
	int PlayEffect(EffectType type, const Vector3& pos, const Vector3& rot = Vector3(), float scale = 20.0f);
	//個別でエフェクトを停止する
	void StopEffect(int handle);
	//エフェクトを停止する
	void StopAll();

	int GetResourceHandle(EffectType type) const;

private:
	//コンストラクタとデストラクタをプライベートにして、シングルトンパターンを実装
	EffectManager() = default;
	virtual ~EffectManager() = default;
	//コピーコンストラクタと代入演算子を削除して、シングルトンのインスタンスが複製されないようにする
	EffectManager(const EffectManager&) = delete;
	EffectManager& operator=(const EffectManager&) = delete;

	//エフェクトの種類ごとのリソースハンドル(未読み込みは-1)
	std::array<int, static_cast<int>(EffectType::Max)> m_resourceHandles;
	std::vector<std::unique_ptr<Effect>> m_effects;
};
