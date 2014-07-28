#define SMAP_SIZE 512
#define SHADOW_EPSILON 0.000005f
static const float4 globalAmbient  = float4(0.1, 0.1, 0.1,0.1);
const float2 aoVec[4] = {float2(1,0),float2(-1,0),  
            float2(0,1),float2(0,-1)};  
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mViewProjection;    //  View * Projection matrix
float4x4 g_mLightVP;			// View * Projection matrix
float4   g_LightDir;
float4   g_LightColor;
float4   g_EyePos;
float4   g_vLightPos;
float    g_specular;				  // �߹����
float    g_fOuterCosTheta;  // Cosine of theta of the spot light
float    g_fInnerCosTheta;

texture DiffuseMap;
texture NormalMap;
texture ShadowMap;
texture PositionMap;

sampler diffuseMap = sampler_state
{
    Texture   = (DiffuseMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
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
    ADDRESSU  = WRAP;
    ADDRESSV  = WRAP;
};

sampler positionMap = sampler_state
{
    Texture   = (PositionMap);
    MinFilter = LINEAR; 
    MagFilter = LINEAR; 
    MipFilter = LINEAR;
	ADDRESSU  = CLAMP;
    ADDRESSV  = CLAMP;
};


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
struct GBuffVS_OUTPUT
{
	float4 Position		 : POSITION0;   // vertex position 
    float2 TextureUV	 : TEXCOORD0;  // vertex texture coord
	float3 InNormalWS	 : TEXCOORD1;
	float3 InBinormalWS  : TEXCOORD2;
    float3 InTangentWS   : TEXCOORD3;
	float4 vPosition     : TEXCOORD4;
};
GBuffVS_OUTPUT GBufferVS(float4 vPos : POSITION0, 
                  float3 vInNormalOS : NORMAL0,
                  float2 vTexCoord0 : TEXCOORD0,
				  float3 vInBinormalOS : BINORMAL,
                  float3 vInTangentOS  : TANGENT
                  )
{
	GBuffVS_OUTPUT outPut;
	outPut.vPosition = mul(vPos,g_mWorld);
	outPut.Position = mul(outPut.vPosition,g_mViewProjection);

	outPut.TextureUV = vTexCoord0;

	outPut.InNormalWS = mul( vInNormalOS,   (float3x3) g_mWorld );
	outPut.InBinormalWS = mul( vInBinormalOS,  (float3x3) g_mWorld );
	outPut.InTangentWS = mul( vInTangentOS, (float3x3) g_mWorld );

	return outPut;
}
struct GBuffPS_OUTPUT
{
	float4 diffuse  : COLOR0;
	float4 normal   : COLOR1;
	float4 position : COLOR2;
};
GBuffPS_OUTPUT GBufferPS(GBuffVS_OUTPUT inPut)
{
	GBuffPS_OUTPUT outPut;
	outPut.diffuse = tex2D(diffuseMap,inPut.TextureUV);
	outPut.position = inPut.vPosition;

	float3 vNormalTS = normalize( tex2D( normalMap, inPut.TextureUV ) * 2 - 1 );
	float3x3 mWorldToTangent = float3x3(inPut.InTangentWS, inPut.InBinormalWS,inPut.InNormalWS );
	float3 normalWS = normalize(mul(vNormalTS,mWorldToTangent));
	outPut.normal = float4(normalWS,1.0);

	return outPut;
}
float doAmbientOcclusion(in float2 tcoord,in float2 uv,in float3 p,in float3 n)
{
	float3 diff = tex2D( positionMap, tcoord + uv) - p;
	float3 v = normalize(diff);
	float d = length(diff)*0.1;
	return max(0,dot(n,v) - 0.1)*(1.0/(1.0+d))*3.0;
}
float4 LightPS(
		in float2 vScreenPosition : TEXCOORD0
		):COLOR
{
	float4 cBaseColor = tex2D( diffuseMap, vScreenPosition );
	float3 vNormal = tex2D( normalMap, vScreenPosition).xyz;
	float4 vPos = tex2D( positionMap, vScreenPosition);
	float4 vPosLight = mul(vPos,g_mLightVP);
	float3 vViewDir = g_EyePos.xyz -  vPos.xyz;
	float3 vLight = vPos.xyz - g_vLightPos.xyz;
	float3 lightDir = g_LightDir.xyz;
	float lightLenSq = vLight.x*vLight.x + vLight.y*vLight.y + vLight.z*vLight.z;
	vLight = normalize(vLight);
	vViewDir = normalize(vViewDir);

	float fshadow = 0.0;
	float lightAmout = 0.0;
	float cosalpha = dot( vLight,  lightDir);
	if( cosalpha >= g_fOuterCosTheta )
	{
		 float depth = vPosLight.z/vPosLight.w;
		 float2 ShadowTexC = 0.5 * vPosLight.xy / vPosLight.w + float2( 0.5, 0.5 );
		 ShadowTexC.y = 1.0f - ShadowTexC.y;

		 float sourcevals[4];
		 sourcevals[0] = tex2D(shadowMap,ShadowTexC) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[1] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,0) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[2] = tex2D(shadowMap,ShadowTexC + float2(0,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;
		 sourcevals[3] = tex2D(shadowMap,ShadowTexC + float2(1.0/SMAP_SIZE,1.0/SMAP_SIZE) ) + SHADOW_EPSILON < depth?0.0f:1.0f;

		 fshadow  = (sourcevals[0] + sourcevals[1] +sourcevals[2] + sourcevals[3])/4;

		 lightAmout = (cosalpha - g_fOuterCosTheta)/(g_fInnerCosTheta - g_fOuterCosTheta);

		 if(cosalpha > g_fInnerCosTheta)
		      lightAmout = 1.0;
	     lightAmout = 1000.0*lightAmout/lightLenSq;
   }
   float ao = 0;
   float rad = 2.0/vPos.z;
   for(int i=0;i<4;i++)
   {
		float2 coord1 = aoVec[i]*rad;
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707, coord1.x*0.707 + coord1.y*0.707);
		ao += doAmbientOcclusion(vScreenPosition,coord1*0.25,vPos,vNormal);
		ao += doAmbientOcclusion(vScreenPosition,coord2*0.5,vPos,vNormal);
		ao += doAmbientOcclusion(vScreenPosition,coord1*0.75,vPos,vNormal);
		ao += doAmbientOcclusion(vScreenPosition,coord2,vPos,vNormal);
   }
   ao /= 16.0;
//   return float4(ao,ao,ao,ao);
   float4 cDiffuse = max(dot(vNormal,-lightDir) ,0)*lightAmout*fshadow*g_LightColor + globalAmbient;

   float4 cSpecular = 0;
   float3 halfvec = vViewDir - lightDir;
   halfvec = normalize(halfvec);
   float fRdotL = saturate(dot(vNormal,halfvec));

   cSpecular =  pow( fRdotL, 100)*g_LightColor;

   float4 cFinalColor = cDiffuse *cBaseColor + cSpecular* fshadow*lightAmout;
   cFinalColor.a = 0.5;
   return cFinalColor* (1.0 - ao);//float4(vNormal,1);
}


technique ShadowTechnique
{
	pass P0
	{
	    VertexShader = compile vs_2_0 shadowVS();
        PixelShader  = compile ps_2_0 shadowPS();
	}
}
technique GBufferTechnique
{
	pass P0
	{
	    VertexShader = compile vs_2_0 GBufferVS();
        PixelShader  = compile ps_2_0 GBufferPS();
	}
}
technique LightTechnique
{
	pass P0
	{
        PixelShader  = compile ps_3_0 LightPS();
	}
}