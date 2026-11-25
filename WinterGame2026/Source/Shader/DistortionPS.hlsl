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

    //横長の画面でも輪が丸くなるように横を伸ばして距離を測る
    float2 d = uv - center;
    d.x *= aspect;
    float dist = length(d);

    //輪の半径は進み具合に合わせて広がる
    //中心がプレイヤーで端に寄っていても、一番遠い画面の角より外まで届く大きさにする
    float radius = progress * 2.2;
    //広がりきる直前まで強さを保ち、最後にすっと消す//画面の端でも波紋が見えるようにする
    float fade = 1.0 - smoothstep(0.75, 1.0, progress);

    //先頭の大きな輪
    float ring = saturate(1.0 - abs(dist - radius) / ringWidth);
    ring *= ring * fade;
    //少し遅れてついてくる細い輪//半分の強さで重ねて波が何重にも走って見えるようにする
    float ring2 = saturate(1.0 - abs(dist - (radius - ringWidth * 1.6)) / (ringWidth * 0.6));
    ring2 *= ring2 * fade * 0.5;
    //まだ輪が出ていない中心(半径が負)では2つ目の輪は出さない
    ring2 *= step(0.0, radius - ringWidth * 1.6);
    float rings = ring + ring2;

    //中心から外向きにずらす
    float2 dir = (dist > 0.0001) ? d / dist : float2(0.0, 0.0);
    dir.x /= aspect;
    float2 offset = dir * rings * strength;

    //ウィッチタイムの間続く揺らぎ
    //プレイヤーを中心にした同心円の波がゆっくり外へ流れる//中心から離れるほど強くする
    float edge = saturate(dist / 0.8);
    edge *= edge;
    float wave = sin(dist * 40.0 - time * 5.0);
    offset += dir * wave * waveStrength * edge;

    //RGBでずらす量を変えて色収差を出す
    float r = sceneTex.Sample(smp, saturate(uv - offset * 1.3)).r;
    float g = sceneTex.Sample(smp, saturate(uv - offset)).g;
    float b = sceneTex.Sample(smp, saturate(uv - offset * 0.7)).b;
    float3 color = float3(r, g, b);
    //ウィッチタイムの色はWitchGradePS.hlslでステージにだけかける//ここではプレイヤーと敵の色を変えない

    //輪のところを薄い紫で光らせる//歪みだけだと平らな床や空の上では波紋が見えにくいため
    //strengthが0なら輪は出ていないので光らない
    float glow = rings * step(0.0001, strength) * 0.35;
    color += float3(0.75, 0.6, 1.0) * glow;

    return float4(saturate(color), 1.0);
}
