struct VSOut
{
    float4 Color : Color;
    float4 Pos : SV_Position;
};

cbuffer MVP
{
    matrix mvp;
};

VSOut main(float3 pos : Position, float4 color : Color)
{
    VSOut outp;
    outp.Pos = mul(float4(pos, 1.0f), mvp);
    outp.Color = color;
    return outp;
}