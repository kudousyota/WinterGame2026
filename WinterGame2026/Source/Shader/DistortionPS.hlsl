//画面の歪み

Texture2D sceneTex : register(t0);
SamplerState smp : register(s0);

//ScreenDistortion::Paramと並びを合わせる
cbuffer Distortion : register(b4)
{
    float2 center;      //波の中心
    float progress;     //0→1で波が広がる
    float strength;     //ずらす強さ
    float aspect;       //画面の幅、高さ
    float ringWidth;    //輪の太さ
    float time;         //揺らぎを動かす時間
    float waveStrength; //揺らぎの強さ
};

struct PSInput
{
    float4 svPos : SV_Position;
    float4 diff : COLOR0;
    float4 spc : COLOR1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    float2 uv = input.uv;

    //横長の画面でも輪が丸くなるように横を伸ばして距離を測る
    float2 d = uv - center;
    d.x *= aspect;
    float dist = length(d);

    //輪の半径は進み具合に合わせて広がる
    float radius = progress * 1.5;
    float ring = saturate(1.0 - abs(dist - radius) / ringWidth);
    ring *= ring;
    //広がるほど弱くする
    ring *= (1.0 - progress);

    //中心から外向きにずらす
    float2 dir = (dist > 0.0001) ? d / dist : float2(0.0, 0.0);
    dir.x /= aspect;
    float2 offset = dir * ring * strength;
    
    //ウィッチタイムの間続く揺らぎ
    //中心から離れるほど強くする
    float edge = saturate(dist / 0.8);
    edge *= edge;
    float2 wave;
    wave.x = sin(uv.y * 25.0f + time * 2.0f);
    wave.y = cos(uv.x * 25.0f + time * 1.8f);
    offset += wave * waveStrength * edge;

    //RGBでずらす量を変えて色収差を出す
    float r = sceneTex.Sample(smp, saturate(uv - offset * 1.3)).r;
    float g = sceneTex.Sample(smp, saturate(uv - offset)).g;
    float b = sceneTex.Sample(smp, saturate(uv - offset * 0.7)).b;

    return float4(r, g, b, 1.0);
}
