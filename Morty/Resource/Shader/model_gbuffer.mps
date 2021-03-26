
struct PS_OUT
{
    float4 f4BaseColor : SV_Target0;
    float4 f4Normal : SV_Target1;
    float fFrontDepth : SV_Target2;
    float fBackDepth : SV_Target3;
}

PS_OUT PS(VS_OUT input) : SV_Target
{
    PS_OUT output;



    return output;
}
    