#define SMAP_SIZE 512
#define SHADOW_EPSILON 0.00005f

float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mViewProjection;    //  View * Projection matrix
float4x4 g_mLightVP;			// View * Projection matrix
float4   g_LightDir;
float4   g_EyePos;
float4   g_vLightPos;
float4	 g_materialAmbientColor;      // Material's ambient color
float4	 g_materialDiffuseColor;      // Material's diffuse color
float4   g_materialSpecularColor;     // Material's specular color
float    g_specular;				  // 高光参数
float    g_heightScale;
float    g_fOuterCosTheta;  // Cosine of theta of the spot light
float    g_fInnerCosTheta;

texture DiffuseMap;
texture NormalMap;
texture ShadowMap;

sampler diffuseMap = sampler_state
{
    Texture   = (DiffuseMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
  //  ADDRESSU  = CLAMP;
  //  ADDRESSV  = CLAMP;
};

sampler shadowMap = sampler_state
{
    Texture   = (ShadowMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
	ADDRESSU  = CLAMP;
    ADDRESSV  = CLAMP;
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
	float4 vPosLight  : TEXCOORD4;
	float4 vPosition  : TEXCOORD5;
};
float4 CaculateColor(float2 texCoord,float3 vLightTS,float3 vViewTS,float fshadow,float lightAmout)
{
	 float4 cBaseColor = tex2D( diffuseMap, texCoord );
	 float3 vNormalTS = normalize( tex2D( normalMap, texCoord ) * 2 - 1 );
	  // Compute diffuse color component:
     float3 vLightTSAdj = float3( -vLightTS.x, vLightTS.y, -vLightTS.z );

	 float4 cDiffuse = ( dot( vNormalTS, vLightTSAdj)) * g_materialDiffuseColor*lightAmout;
	 
	 float4 cSpecular = 0;

	 float3 vReflectionTS = normalize( reflect(vLightTS,vNormalTS) );
	 float fRdotL = saturate( dot( vReflectionTS, vViewTS )) * g_materialSpecularColor;

	 cSpecular = saturate( pow( fRdotL, g_specular ));
	 
	 float4 cFinalColor = (g_materialAmbientColor + cDiffuse) *cBaseColor + cSpecular; 

	 return  cFinalColor*( fshadow + 0.4);
}
VS_OUTPUT mainVS( float4 vPos : POSITION0, 
                  float3 vInNormalOS : NORMAL0,
                  float2 vTexCoord0 : TEXCOORD0,
				  float3 vInBinormalOS : BINORMAL,
                  float3 vInTangentOS  : TANGENT
                  )
{
		VS_OUTPUT outPut;
		outPut.vPosition = mul(vPos,g_mWorld);
		outPut.vPosLight = mul(outPut.vPosition,g_mLightVP);
		outPut.Position = mul(outPut.vPosition,g_mViewProjection);

		outPut.TextureUV = vTexCoord0;
		float3 vNormalWS   = mul( vInNormalOS,   (float3x3) g_mWorld );
		float3 vTangentWS  = mul( vInTangentOS,  (float3x3) g_mWorld );
		float3 vBinormalWS = mul( vInBinormalOS, (float3x3) g_mWorld );

		vNormalWS   = normalize( vNormalWS );
		vTangentWS  = normalize( vTangentWS );
		vBinormalWS = normalize( vBinormalWS );

		float3 vViewWS = g_EyePos - outPut.vPosition;

		float3 vLightWS = g_LightDir;

		float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );
		outPut.LightDirT = mul( mWorldToTangent,vLightWS);
		outPut.ViewDirT = mul( mWorldToTangent,vViewWS);


		float2 vParallaxDirection = normalize(  outPut.ViewDirT.xy );
		float fLength         = length( outPut.ViewDirT );
		float fParallaxLength = sqrt( fLength * fLength - outPut.ViewDirT.z * outPut.ViewDirT.z ) / outPut.ViewDirT.z; 
		outPut.vParallaxOffsetTS = vParallaxDirection * fParallaxLength;
		outPut.vParallaxOffsetTS *= 0.2;


		
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
	 
	 
	 float3 vLight = inPut.vPosition.xyz - g_vLightPos.xyz;
	 float lightLenSq = vLight.x*vLight.x + vLight.y*vLight.y + vLight.z*vLight.z;
	 vLight = normalize(vLight);
	 float fshadow = 0;
	 float lightAmout = 0;
	 float cosalpha = dot( vLight, g_LightDir );
	 if( cosalpha > g_fOuterCosTheta )
	 {
		 float depth = inPut.vPosLight.z/inPut.vPosLight.w;

		 float2 ShadowTexC = 0.5 * inPut.vPosLight.xy / inPut.vPosLight.w + float2( 0.5, 0.5 );
		 ShadowTexC.y = 1.0f - ShadowTexC.y;

		 // transform to texel space
		 float2 texelpos = SMAP_SIZE * ShadowTexC;
        
		 // Determine the lerp amounts           
		 float2 lerps = frac( texelpos );

		 float sourcevals[4];
		 sourcevals[0] = tex2D(shadowMap,ShadowTexC) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[1] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,0) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[2] = tex2D(shadowMap,ShadowTexC + float2(0,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[3] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;

		 fshadow  = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
									  lerp( sourcevals[2], sourcevals[3], lerps.x ),
									  lerps.y );
		 lightAmout = (cosalpha - g_fOuterCosTheta)/(g_fInnerCosTheta - g_fOuterCosTheta);

		 if(cosalpha > g_fInnerCosTheta)
		      lightAmout = 1.0;
		 lightAmout = 1000.0*lightAmout/lightLenSq;
    }
	return CaculateColor(texCoord,vLightTS,vViewTS,fshadow,lightAmout);
}

float4 nmPS(VS_OUTPUT inPut) : COLOR0
{
	 float3 vViewTS   = normalize( inPut.ViewDirT);
     float3 vLightTS  = normalize( inPut.LightDirT);
	 float2 texCoord  = inPut.TextureUV;
	 
	 float3 vLight = inPut.vPosition.xyz - g_vLightPos.xyz;
	 float lightLenSq = vLight.x*vLight.x + vLight.y*vLight.y + vLight.z*vLight.z;
	 vLight = normalize(vLight);
	 float fshadow = 0;
	 float lightAmout = 0;
	 float cosalpha = dot( vLight, g_LightDir );
	 if( cosalpha > g_fOuterCosTheta )
	 {
		 float depth = inPut.vPosLight.z/inPut.vPosLight.w;

		 float2 ShadowTexC = 0.5 * inPut.vPosLight.xy / inPut.vPosLight.w + float2( 0.5, 0.5 );
		 ShadowTexC.y = 1.0f - ShadowTexC.y;

		 // transform to texel space
		 float2 texelpos = SMAP_SIZE * ShadowTexC;
        
		 // Determine the lerp amounts           
		 float2 lerps = frac( texelpos );

		 float sourcevals[4];
		 sourcevals[0] = tex2D(shadowMap,ShadowTexC) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[1] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,0) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[2] = tex2D(shadowMap,ShadowTexC + float2(0,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[3] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;

		 fshadow  = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
									  lerp( sourcevals[2], sourcevals[3], lerps.x ),
									  lerps.y );
		 lightAmout = (cosalpha - g_fOuterCosTheta)/(g_fInnerCosTheta - g_fOuterCosTheta);

		 if(cosalpha > g_fInnerCosTheta)
		      lightAmout = 1.0;
		 lightAmout = 1000.0*lightAmout/lightLenSq;
    }
	return CaculateColor(texCoord,vLightTS,vViewTS,fshadow,lightAmout);
}

void shadowVS(float4 vPos : POSITION0,
			  out float4 oPos:POSITION,
			  out float2 Depth:TEXCOORD0)
{
	float4 vPosition = mul(vPos,g_mWorld);
	oPos = mul(vPosition,g_mLightVP);

	Depth.xy = oPos.zw;
}
float4 shadowPS(float2 Depth:TEXCOORD0) : COLOR0
{
	float4 color;
	color.rgb = float3(1,1,1);
	return color.a = Depth.x/Depth.y;
}

technique ShadowTechnique
{
	pass P0
	{
	    VertexShader = compile vs_2_0 shadowVS();
        PixelShader  = compile ps_2_0 shadowPS();
	}
}
technique NMTechnique
{
	pass P0
	{
	    VertexShader = compile vs_3_0 mainVS();
        PixelShader  = compile ps_3_0 nmPS();
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