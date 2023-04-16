#include "tonemap.hlsl"

[[vk::binding(1,0)]]Texture2D u_texScreenTexture;
[[vk::binding(2,0)]]sampler LinearSampler;


//[[vk::binding(3,0)]]cbuffer cbColorGrading
//{
//    float ExpandGamut;
//};


struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};


static const float3x3 GamutMappingIdentityMatrix = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };

//
// Gamut conversion matrices
//
float3x3 OuputGamutMappingMatrix( uint OutputGamut )
{
	// Gamut mapping matrices used later
	const float3x3 AP1_2_sRGB    = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );
	const float3x3 AP1_2_DCI_D65 = mul( XYZ_2_P3D65_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );
	const float3x3 AP1_2_Rec2020 = mul( XYZ_2_Rec2020_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );

	// Set gamut mapping matrix 
	// 0 = sRGB - D65
	// 1 = P3 - D65
	// 2 = Rec.2020 - D65
	// 3 = ACES AP0 - D60
	// 4 = ACES AP1 - D60

	if( OutputGamut == 1 )
		return AP1_2_DCI_D65;
	else if( OutputGamut == 2 )
		return AP1_2_Rec2020;
	else if( OutputGamut == 3 )
		return AP1_2_AP0_MAT;
	else if( OutputGamut == 4 )
		return GamutMappingIdentityMatrix;
	else
		return AP1_2_sRGB;
}


float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
    float ExpandGamut = 1.0f;
    float BlueCorrection = 0.6f;
    float ToneCurveAmount = 0.0f;
    
	const float3x3 sRGB_2_AP1 = mul( XYZ_2_AP1_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );
	const float3x3 AP1_2_sRGB = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );

	const float3x3 AP0_2_AP1 = mul( XYZ_2_AP1_MAT, AP0_2_XYZ_MAT );
	const float3x3 AP1_2_AP0 = mul( XYZ_2_AP0_MAT, AP1_2_XYZ_MAT );

	//const float3x3 AP1_2_Output  = OuputGamutMappingMatrix( OutputGamut );

    //Linear color
    float4 color = u_texScreenTexture.Sample(LinearSampler, input.uv);

    //Convert to AP1
	float3 ColorAP1 = mul( sRGB_2_AP1, color.rgb );

    

	// Expand bright saturated colors outside the sRGB gamut to fake wide gamut rendering.
 	float  LumaAP1 = dot( ColorAP1, AP1_RGB2Y );
	float3 ChromaAP1 = ColorAP1 / LumaAP1;

	float ChromaDistSqr = dot( ChromaAP1 - 1, ChromaAP1 - 1 );
	float ExpandAmount = ( 1 - exp2( -4 * ChromaDistSqr ) ) * ( 1 - exp2( -4 * ExpandGamut * LumaAP1*LumaAP1 ) );

	// Bizarre matrix but this expands sRGB to between P3 and AP1
	// CIE 1931 chromaticities:	x		y
	//				Red:		0.6965	0.3065
	//				Green:		0.245	0.718
	//				Blue:		0.1302	0.0456
	//				White:		0.3127	0.329
	const float3x3 Wide_2_XYZ_MAT = 
	{
		0.5441691,  0.2395926,  0.1666943,
		0.2394656,  0.7021530,  0.0583814,
		-0.0023439,  0.0361834,  1.0552183,
	};

	const float3x3 Wide_2_AP1 = mul( XYZ_2_AP1_MAT, Wide_2_XYZ_MAT );
	const float3x3 ExpandMat = mul( Wide_2_AP1, AP1_2_sRGB );

	float3 ColorExpand = mul( ExpandMat, ColorAP1 );
	ColorAP1 = lerp( ColorAP1, ColorExpand, ExpandAmount );

//Color Grading
	//ColorAP1 = ColorCorrectAll( ColorAP1 );

	// Store for Linear HDR output without tone curve
	float3 GradedColor = mul( AP1_2_sRGB, ColorAP1 );



//ACES LMT + RRT + ODT， BlueLightArtifact Fix
	const float3x3 BlueCorrect =
	{
		0.9404372683, -0.0183068787, 0.0778696104,
		0.0083786969,  0.8286599939, 0.1629613092,
		0.0005471261, -0.0008833746, 1.0003362486
	};
	const float3x3 BlueCorrectInv =
	{
		1.06318,     0.0233956, -0.0865726,
		-0.0106337,   1.20632,   -0.19569,
		-0.000590887, 0.00105248, 0.999538
	};
	const float3x3 BlueCorrectAP1    = mul( AP0_2_AP1, mul( BlueCorrect,    AP1_2_AP0 ) );
	const float3x3 BlueCorrectInvAP1 = mul( AP0_2_AP1, mul( BlueCorrectInv, AP1_2_AP0 ) );

	// Blue correction
	ColorAP1 = lerp( ColorAP1, mul( BlueCorrectAP1, ColorAP1 ), BlueCorrection );
//End



    // Tonemapped color in the AP1 gamut
	float3 ToneMappedColorAP1 = FilmToneMap( ColorAP1 );
	ColorAP1 = lerp(ColorAP1, ToneMappedColorAP1, ToneCurveAmount);

	// Uncorrect blue to maintain white point
	ColorAP1 = lerp( ColorAP1, mul( BlueCorrectInvAP1, ColorAP1 ), BlueCorrection );

	// Convert from AP1 to sRGB and clip out-of-gamut values
	float3 FilmColor = max(0, mul( AP1_2_sRGB, ColorAP1 ));


    // apply math color correction on top to texture based solution
	//FilmColor = ColorCorrection( FilmColor );

	// blend with custom LDR color, used for Fade track in Cinematics
	//float3 FilmColorNoGamma = lerp( FilmColor * ColorScale, OverlayColor.rgb, OverlayColor.a );
	// Apply Fade track to linear outputs also
	//GradedColor = lerp(GradedColor * ColorScale, OverlayColor.rgb, OverlayColor.a);


	// Apply "gamma" curve adjustment.
	//FilmColor = pow( max(0, FilmColorNoGamma), InverseGamma.y );


    // This is different than the prior "gamma" curve adjustment (but reusing the variable).
    // For displays set to a gamma colorspace.
    // Note, MacOSX native output is raw gamma 2.2 not sRGB!
    //float3 OutDeviceColor = pow( OutputGamutColor, InverseGamma.z );

	// Better to saturate(lerp(a,b,t)) than lerp(saturate(a),saturate(b),t)
	//float3 OutColor = OutDeviceColor / 1.05;


    return float4(FilmColor, color.a);
}