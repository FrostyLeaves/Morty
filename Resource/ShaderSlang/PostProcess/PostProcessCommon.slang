implementing PostProcessModule;

public struct PostProcessVertex
{
    float2 pos : POSITION;
}

public struct PostProcessPixel
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
}

public struct FullScreenQuad
{
    public static PostProcessPixel GetVertex(PostProcessVertex input)
    {
        PostProcessPixel output;
        output.pos = float4(input.pos.xy, 0.0, 1.0);
        output.uv.x = input.pos.x * 0.5 + 0.5;
        output.uv.y = 0.5 - input.pos.y * 0.5;
        return output;
    }
}
