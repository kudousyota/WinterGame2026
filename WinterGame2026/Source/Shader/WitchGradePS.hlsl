//ウィッチタイムの背景の色
//ステージを描いた直後の画面にだけかける//プレイヤーと敵はこの後に描くので色は変わらない

Texture2D sceneTex : register(t0);
SamplerState smp : register(s0);

//ScreenDistortion::Paramと並びを合わせる//DistortionPS.hlslと同じ並び
cbuffer Distortion : register(b4)
{
    float2 center;      //波の中心
    float progress;     //0→1で波が広がる
    float strength;     //ずらす強さ
    float aspect;       //画面の幅、高さ
    float ringWidth;    //輪の太さ
    float time;         //揺らぎを動かす時間
    float waveStrength; //揺らぎの強さ
    float gradeAmount;  //ウィッチタイムの色にする割合//0で元の色、1で暗い藍色と紫
    float gradeRadius;  //この半径より内側だけ色を変える//波紋が通ったところから色が変わるようにする
    float vignette;     //画面の四隅を暗くする強さ
    float padding;      //16バイトにそろえるための空き
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
    float3 color = sceneTex.Sample(smp, uv).rgb;

    //明るさだけを取り出して、暗いところは黒に近い藍色、明るいところは薄い紫に塗り直す
    //元の色も少し残して、真っ平らな色にならないようにする
    float luma = dot(color, float3(0.299, 0.587, 0.114));
    float3 darkColor = float3(0.02, 0.01, 0.07);
    float3 lightColor = float3(0.72, 0.62, 1.0);
    float3 graded = lerp(darkColor, lightColor, luma);
    graded = lerp(graded, color, 0.2);

    //四隅を暗くする//中心からの距離はプレイヤーではなく画面の真ん中で測る
    float2 v = (uv - 0.5) * float2(aspect, 1.0);
    float vig = saturate(1.0 - dot(v, v) * 0.9);
    graded *= lerp(1.0, vig, vignette);

    //波紋が通ったところ(半径の内側)だけ色を変える//境目は少しぼかす
    //横長の画面でも丸くなるように横を伸ばして距離を測る//波紋の輪と同じ測り方
    float2 d = uv - center;
    d.x *= aspect;
    float dist = length(d);
    float mask = saturate((gradeRadius - dist) / 0.08);

    return float4(lerp(color, graded, gradeAmount * mask), 1.0);
}
