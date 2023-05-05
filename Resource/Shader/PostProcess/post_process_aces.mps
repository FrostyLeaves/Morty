#include "tonemap.hlsl"

[[vk::binding(1,0)]]Texture2D u_texScreenTexture;
[[vk::binding(2,0)]]sampler LinearSampler;


[[vk::binding(3,0)]]cbuffer cbColorGrading
{
	uint bIsTemperatureWhiteBalance;

	float WhiteTemp;
	float WhiteTint;

	// Color Correction controls
	float4 ColorSaturation;
	float4 ColorContrast;
	float4 ColorGamma;
	float4 ColorGain;
	float4 ColorOffset;

	float4 ColorSaturationShadows;
	float4 ColorContrastShadows;
	float4 ColorGammaShadows;
	float4 ColorGainShadows;
	float4 ColorOffsetShadows;

	float4 ColorSaturationMidtones;
	float4 ColorContrastMidtones;
	float4 ColorGammaMidtones;
	float4 ColorGainMidtones;
	float4 ColorOffsetMidtones;

	float4 ColorSaturationHighlights;
	float4 ColorContrastHighlights;
	float4 ColorGammaHighlights;
	float4 ColorGainHighlights;
	float4 ColorOffsetHighlights;

	float ColorCorrectionShadowsMax;
	float ColorCorrectionHighlightsMin;
	float ColorCorrectionHighlightsMax;

	float BlueCorrection;
	float ExpandGamut;
	float ToneCurveAmount;


	float3 MappingPolynomial;
	float4 ColorScale;
	float4 OverlayColor;

	float3 InverseGamma;
};




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

float3 ColorCorrect( float3 WorkingColor,
	float4 ColorSaturation,
	float4 ColorContrast,
	float4 ColorGamma,
	float4 ColorGain,
	float4 ColorOffset )
{
	// TODO optimize
	float Luma = dot( WorkingColor, AP1_RGB2Y );
	WorkingColor = max( 0, lerp( Luma.xxx, WorkingColor, ColorSaturation.xyz*ColorSaturation.w ) );
	WorkingColor = pow( WorkingColor * (1.0 / 0.18), ColorContrast.xyz*ColorContrast.w ) * 0.18;
	WorkingColor = pow( WorkingColor, 1.0 / (ColorGamma.xyz*ColorGamma.w) );
	WorkingColor = WorkingColor * (ColorGain.xyz * ColorGain.w) + (ColorOffset.xyz + ColorOffset.w);
	return WorkingColor;
}

// Nuke-style Color Correct
float3 ColorCorrectAll( float3 WorkingColor )
{
	float Luma = dot( WorkingColor, AP1_RGB2Y );

	// Shadow CC
	float3 CCColorShadows = ColorCorrect(WorkingColor, 
		ColorSaturationShadows*ColorSaturation, 
		ColorContrastShadows*ColorContrast, 
		ColorGammaShadows*ColorGamma, 
		ColorGainShadows*ColorGain, 
		ColorOffsetShadows+ColorOffset);
	float CCWeightShadows = 1- smoothstep(0, ColorCorrectionShadowsMax, Luma);
	
	// Highlight CC
	float3 CCColorHighlights = ColorCorrect(WorkingColor, 
		ColorSaturationHighlights*ColorSaturation, 
		ColorContrastHighlights*ColorContrast, 
		ColorGammaHighlights*ColorGamma, 
		ColorGainHighlights*ColorGain, 
		ColorOffsetHighlights+ColorOffset);
	float CCWeightHighlights = smoothstep(ColorCorrectionHighlightsMin, ColorCorrectionHighlightsMax, Luma);

	// Midtone CC
	float3 CCColorMidtones = ColorCorrect(WorkingColor, 
		ColorSaturationMidtones*ColorSaturation, 
		ColorContrastMidtones*ColorContrast, 
		ColorGammaMidtones*ColorGamma, 
		ColorGainMidtones*ColorGain, 
		ColorOffsetMidtones+ColorOffset);
	float CCWeightMidtones = 1 - CCWeightShadows - CCWeightHighlights;

	// Blend Shadow, Midtone and Highlight CCs
	float3 WorkingColorSMH = CCColorShadows*CCWeightShadows + CCColorMidtones*CCWeightMidtones + CCColorHighlights*CCWeightHighlights;
	
	return WorkingColorSMH;
}

// Accurate for 4000K < Temp < 25000K
// in: correlated color temperature
// out: CIE 1931 chromaticity
float2 D_IlluminantChromaticity( float Temp )
{
	// Correct for revision of Plank's law
	// This makes 6500 == D65
	Temp *= 1.4388 / 1.438;
	float OneOverTemp = 1.0/Temp;
	float x =	Temp <= 7000 ?
				0.244063 + ( 0.09911e3 + ( 2.9678e6 - 4.6070e9 * OneOverTemp ) * OneOverTemp) * OneOverTemp:
				0.237040 + ( 0.24748e3 + ( 1.9018e6 - 2.0064e9 * OneOverTemp ) * OneOverTemp ) * OneOverTemp;
	
	float y = -3 * x*x + 2.87 * x - 0.275;

	return float2(x,y);
}

// Accurate for 1000K < Temp < 15000K
// [Krystek 1985, "An algorithm to calculate correlated colour temperature"]
float2 PlanckianLocusChromaticity( float Temp )
{
	float u = ( 0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp*Temp ) / ( 1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp*Temp );
	float v = ( 0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp*Temp ) / ( 1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp*Temp );

	float x = 3*u / ( 2*u - 8*v + 4 );
	float y = 2*v / ( 2*u - 8*v + 4 );

	return float2(x,y);
}

float2 PlanckianIsothermal( float Temp, float Tint )
{
	float u = ( 0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp*Temp ) / ( 1.0f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp*Temp );
	float v = ( 0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp*Temp ) / ( 1.0f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp*Temp );

	float ud = ( -1.13758118e9f - 1.91615621e6f * Temp - 1.53177f * Temp*Temp ) / Square( 1.41213984e6f + 1189.62f * Temp + Temp*Temp );
	float vd = (  1.97471536e9f - 705674.0f * Temp - 308.607f * Temp*Temp ) / Square( 6.19363586e6f - 179.456f * Temp + Temp*Temp );

	float2 uvd = normalize( float2( u, v ) );

	// Correlated color temperature is meaningful within +/- 0.05
	u += -uvd.y * Tint * 0.05;
	v +=  uvd.x * Tint * 0.05;
	
	float x = 3*u / ( 2*u - 8*v + 4 );
	float y = 2*v / ( 2*u - 8*v + 4 );

	return float2(x,y);
}

// @param InLDRColor needs to be LDR (0..1) and in linear space
half3 ColorCorrection(half3 InLDRColor)
{
	// final color correction to adjust for hardware differences, to make quick adjustements before a demo or simply a user setting
	return MappingPolynomial.x * (InLDRColor * InLDRColor) + MappingPolynomial.y * InLDRColor + MappingPolynomial.z;
}

float3 WhiteBalance( float3 LinearColor )
{
	float2 SrcWhiteDaylight = D_IlluminantChromaticity( WhiteTemp );
	float2 SrcWhitePlankian = PlanckianLocusChromaticity( WhiteTemp );

	float2 SrcWhite = WhiteTemp < 4000 ? SrcWhitePlankian : SrcWhiteDaylight;
	float2 D65White = float2( 0.31270,  0.32900 );

	{
		// Offset along isotherm
		float2 Isothermal = PlanckianIsothermal( WhiteTemp, WhiteTint ) - SrcWhitePlankian;
		SrcWhite += Isothermal;
	}

	if (!bIsTemperatureWhiteBalance)
	{
		float2 Temp = SrcWhite;
		SrcWhite = D65White;
		D65White = Temp;
	}

	float3x3 WhiteBalanceMat = ChromaticAdaptation( SrcWhite, D65White );
	WhiteBalanceMat = mul( XYZ_2_sRGB_MAT, mul( WhiteBalanceMat, sRGB_2_XYZ_MAT ) );

	return mul( WhiteBalanceMat, LinearColor );
}

half LinearToSrgbBranchingChannel(half lin) 
{
	if(lin < 0.00313067) return lin * 12.92;
	return pow(lin, (1.0/2.4)) * 1.055 - 0.055;
}

half3 LinearToSrgbBranching(half3 lin) 
{
	return half3(
		LinearToSrgbBranchingChannel(lin.r),
		LinearToSrgbBranchingChannel(lin.g),
		LinearToSrgbBranchingChannel(lin.b));
}

half3 LinearToSrgb(half3 lin) 
{
	return LinearToSrgbBranching(lin);
}

float4 PS_MAIN(VS_OUT_POST input) : SV_Target
{
	
    
	const float3x3 sRGB_2_AP1 = mul( XYZ_2_AP1_MAT, mul( D65_2_D60_CAT, sRGB_2_XYZ_MAT ) );
	const float3x3 AP1_2_sRGB = mul( XYZ_2_sRGB_MAT, mul( D60_2_D65_CAT, AP1_2_XYZ_MAT ) );

	const float3x3 AP0_2_AP1 = mul( XYZ_2_AP1_MAT, AP0_2_XYZ_MAT );
	const float3x3 AP1_2_AP0 = mul( XYZ_2_AP0_MAT, AP1_2_XYZ_MAT );

	//output srgb
	const float3x3 AP1_2_Output  = OuputGamutMappingMatrix( 5 );

    //Linear color				sRGB线性颜色
    float4 f4LinearColor = u_texScreenTexture.Sample(LinearSampler, input.uv);

	//White balance
	float3 f3BalancedColor = WhiteBalance( f4LinearColor.rgb );

    //Convert to AP1			转换到AP1空间下
	float3 ColorAP1 = mul( sRGB_2_AP1, f3BalancedColor );

    

	// Expand bright saturated colors outside the sRGB gamut to fake wide gamut rendering.

	//AP1空间下的亮度
 	float  LumaAP1 = dot( ColorAP1, AP1_RGB2Y );

	//色度
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
	ColorAP1 = ColorCorrectAll( ColorAP1 );

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
	FilmColor = ColorCorrection( FilmColor );

	// blend with custom LDR color, used for Fade track in Cinematics
	float3 FilmColorNoGamma = lerp( FilmColor * ColorScale.rgb, OverlayColor.rgb, OverlayColor.a );
	// Apply Fade track to linear outputs also
	GradedColor = lerp(GradedColor * ColorScale.rgb, OverlayColor.rgb, OverlayColor.a);

	// Apply "gamma" curve adjustment.
	FilmColor = pow( max(0, FilmColorNoGamma), InverseGamma.y );

	// Convert from sRGB to specified output gamut	
	//float3 OutputGamutColor = mul( AP1_2_Output, mul( sRGB_2_AP1, FilmColor ) );

	// FIXME: Workaround for UE-29935, pushing all colors with a 0 component to black output
	// Default parameters seem to cancel out (sRGB->XYZ->AP1->XYZ->sRGB), so should be okay for a temp fix
	float3 OutputGamutColor = FilmColor;

	// Apply conversion to sRGB (this must be an exact sRGB conversion else darks are bad).
	float3 OutDeviceColor = LinearToSrgb( OutputGamutColor );

	//Better to saturate(lerp(a,b,t)) than lerp(saturate(a),saturate(b),t)
	float3 OutColor = OutDeviceColor / 1.05;

    return float4(OutColor.rgb, f4LinearColor.a);
}