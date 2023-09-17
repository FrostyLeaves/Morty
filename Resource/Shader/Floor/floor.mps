#include "Internal/internal_constant.hlsl"

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 near_pos : NEAR_POS;
    float3 far_pos : FAR_POS;
};

float GetGridLine(VS_OUTPUT input, float3 position, float fCurrentGridLevel)
{
    float fCurrentGridDistance = pow(2, fCurrentGridLevel);
    float2 f2CurrentCoord = position.xz / fCurrentGridDistance;
    float2 derivative = fwidth(f2CurrentCoord);
    float2 f2CurrentGrid = abs(frac(f2CurrentCoord - 0.5) - 0.5) / derivative;
    float fCurrentGridLine = min(f2CurrentGrid.x, f2CurrentGrid.y);
    
    return fCurrentGridLine;
}

float4 PS_MAIN(VS_OUTPUT input) : SV_Target
{
    float t = -input.near_pos.y / (input.far_pos.y - input.near_pos.y);
    float3 position = input.near_pos + t * (input.far_pos - input.near_pos);

    if (t < 0)
    {
        discard;
    }

    float fCameraHeight = u_f3CameraPosition.y;
    float fMinGridDistance = 4;
    float fCameraHeightLevel = log2(abs(fCameraHeight));
    float fInterpolation = fCameraHeightLevel - floor(fCameraHeightLevel);
    
    float fCurrentGridLevel = max(fMinGridDistance, floor(fCameraHeightLevel) - 2);
    float fNextGridLevel = max(fMinGridDistance, floor(fCameraHeightLevel) - 1);
    
    float fCurrentGridLine = GetGridLine(input, position, fCurrentGridLevel);
    float fNextGridLine = GetGridLine(input, position, fNextGridLevel);

    float fClipSpaceDepth = input.pos.z;
    float near = u_matZNearFar.x;
    float far = u_matZNearFar.y;
    float fLinearDepth = (2.0 * near * far) / (far + near - fClipSpaceDepth * (far - near)) / far;

    float minimumz = max(0.01, fLinearDepth);
    float minimumx = max(0.01, fLinearDepth);

    float4 color = float4(1.0, 1.0, 1.0, 1.0);
    color.a = color.a - min(fCurrentGridLine, 1.0);
    
    if (distance(fCurrentGridLine, fNextGridLine) >= 1e-3)
    {
        color.a *= 1.0 - fInterpolation;
    }
    
    if (color.a < 0.1)
    {
        discard;
    }

    // z axis
    if(abs(position.x) < minimumz)
    {
        color = float4(0.0, 0.0, 1.0, 1.0);
    }
    // x axis
    if(abs(position.z) < minimumx)
    {
        color = float4(1.0, 0.0, 0.0, 1.0);
    }

    float fFading = max(0, (1.0 - fLinearDepth));
    color.a = color.a * fFading / (t * t);

    return color;
}
