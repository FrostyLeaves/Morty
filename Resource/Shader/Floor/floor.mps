#include "../inner_constant.hlsl"

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 near_pos : NEAR_POS;
    float3 far_pos : FAR_POS;
};

float4 PS_MAIN(VS_OUTPUT input) : SV_Target
{
    float t = -input.near_pos.y / (input.far_pos.y - input.near_pos.y);
    if (t < 0)
    {
        discard;
    }

    float fClipSpaceDepth = input.pos.z;
    float near = u_matZNearFar.x;
    float far = u_matZNearFar.y;
    float fLinearDepth = (2.0 * near * far) / (far + near - fClipSpaceDepth * (far - near)) / far;

    float3 position = input.near_pos + t * (input.far_pos - input.near_pos);
    float scale = 1;

    float2 coord = position.xz * scale; // use the scale variable to set the distance between the lines
    float2 derivative = fwidth(coord);
    float2 grid = abs(frac(coord - 0.5) - 0.5) / derivative;
    float fGridLine = min(grid.x, grid.y);
    
    //float minimumz = min(derivative.y, 0.1);
    //float minimumx = min(derivative.x, 0.1);
    
    float minimumz = max(0.01, fLinearDepth);
    float minimumx = max(0.01, fLinearDepth);

    float4 color = float4(1, 1, 1, 1);

    color.a = color.a - min(fGridLine, 1.0);
    
    if (color.a < 0.1)
    {
        discard;
    }

    // z axis
    if(abs(position.x) < minimumz)
    {
        return float4(0.0, 0.0, 1.0, 1.0);
    }
    // x axis
    if(abs(position.z) < minimumx)
    {
        return float4(1.0, 0.0, 0.0, 1.0);
    }

    float fFading = max(0, (1.0 - fLinearDepth));
    color.a = color.a * fFading / (t * t);

    return color;
}
