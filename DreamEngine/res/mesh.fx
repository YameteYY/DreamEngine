float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4   g_LightDir;
float4   g_EyePos;
float4	 g_materialAmbientColor;      // Material's ambient color
float4	 g_materialDiffuseColor;      // Material's diffuse color
float4   g_materialSpecularColor;     // Material's specular color

texture DiffuseMap;
texture NormalMap;

sampler diffuseMap = sampler_state
{
    Texture   = (DiffuseMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
  //  ADDRESSU  = CLAMP;
  //  ADDRESSV  = CLAMP;
};

sampler normalMap = sampler_state
{
    Texture   = (NormalMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
  //  ADDRESSU  = CLAMP;
 //   ADDRESSV  = CLAMP;
};

struct VS_OUTPUT
{
    float4 Position   : POSITION0;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coord
	float3 LightDirT  : TEXCOORD1;
	float3 ViewDirT	  : TEXCOORD2;
	float2 vParallaxOffsetTS : TEXCOORD3;
};
float4 CaculateColor(float2 texCoord,float3 vLightTS,float3 vViewTS,float fshadow)
{
	 float4 cBaseColor = tex2D( diffuseMap, texCoord );
	 float3 vNormalTS = normalize( tex2D( normalMap, texCoord ) * 2 - 1 );
	 float4 cDiffuse = saturate( dot( vNormalTS, -vLightTS)) * g_materialDiffuseColor;
	 
	 float4 cSpecular = 0;

	 float3 vReflectionTS = normalize( reflect(vLightTS,vNormalTS) );
	 float fRdotL = saturate( dot( vReflectionTS, vViewTS )) * g_materialSpecularColor;

	 cSpecular = saturate( pow( fRdotL, 10.0 ));
	 
	 float4 cFinalColor = (g_materialAmbientColor + cDiffuse) *cBaseColor + cSpecular; 

	 return  cFinalColor*fshadow;
}
VS_OUTPUT mainVS( float4 vPos : POSITION0, 
                  float3 vInNormalOS : NORMAL0,
                  float2 vTexCoord0 : TEXCOORD0,
				  				float3 vInBinormalOS : BINORMAL,
                  float3 vInTangentOS  : TANGENT
                  )
{
		VS_OUTPUT outPut;
		float4 vPosition = mul(vPos,g_mWorld);
		outPut.Position = mul(vPosition,g_mWorldViewProjection);

		outPut.TextureUV = vTexCoord0;
		float3 vNormalWS   = mul( vInNormalOS,   (float3x3) g_mWorld );
		float3 vTangentWS  = mul( vInTangentOS,  (float3x3) g_mWorld );
		float3 vBinormalWS = mul( vInBinormalOS, (float3x3) g_mWorld );

		vNormalWS   = normalize( vNormalWS );
		vTangentWS  = normalize( vTangentWS );
		vBinormalWS = normalize( vBinormalWS );

		float3 vViewWS = g_EyePos - vPosition;

		float3 vLightWS = g_LightDir;

		float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );
		outPut.LightDirT = mul( mWorldToTangent,vLightWS);
		outPut.ViewDirT = mul( mWorldToTangent,vViewWS);


		float2 vParallaxDirection = normalize(  outPut.ViewDirT.xy );
		float fLength         = length( outPut.ViewDirT );
		float fParallaxLength = sqrt( fLength * fLength - outPut.ViewDirT.z * outPut.ViewDirT.z ) / outPut.ViewDirT.z; 
		outPut.vParallaxOffsetTS = vParallaxDirection * fParallaxLength;
		outPut.vParallaxOffsetTS *= 0.1;
		return outPut;
}
float4 pmPS(VS_OUTPUT inPut) : COLOR0
{
	 int  nNumSteps = 50;
	 float3 vViewTS   = normalize( inPut.ViewDirT);
     float3 vLightTS  = normalize( inPut.LightDirT);

	 float2 texCoord = inPut.TextureUV;
	 float fCurrHeight = 0.0;
	 float fStepSize = 1.0 / (float)nNumSteps;
	 float fPrevHeight = 0.0;

	 int nStepIndex = 0;
	 float2 vTexOffsetPerStep = fStepSize * inPut.vParallaxOffsetTS;
	 float2 vTexCurrentOffset = texCoord;
	 float fCurrentBound = 1.0;
	 float fParallaxAmount = 0.0;
	 float2 pt1 = 0;
	 float2 pt2 = 0;
	 float2 texOffset2 = 0;

	 while(nStepIndex < nNumSteps)
	 {
		vTexCurrentOffset -= vTexOffsetPerStep;

		fCurrHeight = tex2D(normalMap,vTexCurrentOffset).a;

		fCurrentBound -= fStepSize;

		if(fCurrHeight > fCurrentBound)
		{
			pt1 = float2(fCurrentBound,fCurrHeight);
			pt2 = float2(fCurrentBound + fStepSize,fPrevHeight);

			texOffset2 = vTexCurrentOffset - vTexOffsetPerStep;

			nStepIndex = nNumSteps + 1;
			fPrevHeight = fCurrHeight;
		}
		else
		{
			nStepIndex++;
			fPrevHeight = fCurrHeight;
		}
	 }
	  
	 float fDelta2 = pt2.x - pt2.y;  
     float fDelta1 = pt1.x - pt1.y;  
	 float fDenominator = fDelta2 - fDelta1;
	 if ( fDenominator == 0.0f )
     {
        fParallaxAmount = 0.0f;
     }
     else
     {
        fParallaxAmount = (pt1.x * fDelta2 - pt2.x * fDelta1 ) / fDenominator;
     }
	 float2 vParallaxOffset = inPut.vParallaxOffsetTS * (1 - fParallaxAmount );
	 
	 texCoord -= vParallaxOffset;
	 
	 return CaculateColor(texCoord,vLightTS,vViewTS,1.0);
}

float4 nmPS(VS_OUTPUT inPut) : COLOR0
{
	 float3 vViewTS   = normalize( inPut.ViewDirT);
     float3 vLightTS  = normalize( inPut.LightDirT);
	 float2 texCoord  = inPut.TextureUV;
	 
	 return CaculateColor(texCoord,vLightTS,vViewTS,1.0);
}

technique NMTechnique
{
	pass P0
	{
	    VertexShader = compile vs_2_0 mainVS();
        PixelShader  = compile ps_2_0 nmPS();
	}
}
technique PMTechnique
{
	pass P0
	{
	    VertexShader = compile vs_3_0 mainVS();
        PixelShader  = compile ps_3_0 pmPS();
	}
}