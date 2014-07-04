float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix
float4   g_LightDir;
float4   g_EyePos;

texture DiffuseMap;
texture NormalMap;

sampler diffuseMap = sampler_state
{
    Texture   = (DiffuseMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
};

sampler normalMap = sampler_state
{
    Texture   = (NormalMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
};

struct VS_OUTPUT
{
    float4 Position   : POSITION0;   // vertex position 
    float2 TextureUV  : TEXCOORD0;  // vertex texture coord
	float3 LightDirT  : TEXCOORD1;
	float3 ViewDirT	  : TEXCOORD2;
};
VS_OUTPUT mainVS( float4 vPos : POSITION0, 
                  float3 vInNormalOS : NORMAL0,
                  float2 vTexCoord0 : TEXCOORD0,
				  float3 vInBinormalOS : BINORMAL,
                  float3 vInTangentOS  : TANGENT
                  )
{
		VS_OUTPUT outPut;
		outPut.Position = mul(vPos,g_mWorld);
		
		outPut.TextureUV = vTexCoord0;
		float3 vNormalWS   = mul( vInNormalOS,   (float3x3) g_mWorld );
		float3 vTangentWS  = mul( vInTangentOS,  (float3x3) g_mWorld );
		float3 vBinormalWS = mul( vInBinormalOS, (float3x3) g_mWorld );

		vNormalWS   = normalize( vNormalWS );
		vTangentWS  = normalize( vTangentWS );
		vBinormalWS = normalize( vBinormalWS );

		float3 vViewWS = g_EyePos - outPut.Position;

		float3 vLightWS = g_LightDir;

		float3x3 mWorldToTangent = float3x3( vTangentWS, vBinormalWS, vNormalWS );
		outPut.LightDirT = mul( mWorldToTangent,g_LightDir);
		outPut.ViewDirT = mul( mWorldToTangent,vViewWS);

		outPut.Position = mul(outPut.Position,g_mWorldViewProjection);
		return outPut;
}


float4 mainPS(VS_OUTPUT inPut) : COLOR0
{

	 float3 vViewTS   = normalize( inPut.ViewDirT);
     float3 vLightTS  = normalize( inPut.LightDirT);

	 float4 cResultColor = float4( 0, 0, 0, 1 );
	 float2 texCoord = inPut.TextureUV;

	 float3 vNormalTS = normalize( tex2D( normalMap, texCoord ) * 2 - 1 );
	 float4 cBaseColor = tex2D( diffuseMap, texCoord );

	 float4 cDiffuse = saturate( dot( vNormalTS, -vLightTS));
	 
	 float4 cSpecular = 0;

	 float3 vReflectionTS = normalize( reflect(vLightTS,vNormalTS) );
	 float fRdotL = saturate( dot( vReflectionTS, vViewTS ));

	 cSpecular = saturate( pow( fRdotL, 10.0 ));
	 
	 float4 cFinalColor = cDiffuse *cBaseColor*2.0 + cSpecular; 

	 return cFinalColor;
}

technique SBTechnique
{
	pass P0
	{
	  VertexShader = compile vs_2_0 mainVS();
      PixelShader  = compile ps_2_0 mainPS();
	}
}