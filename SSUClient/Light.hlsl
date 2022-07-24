//--------------------------------------------------------------------------------------
#define MAX_LIGHTS			16 
#define MAX_MATERIALS		16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

#define FRAME_BUFFER_WIDTH		1800
#define FRAME_BUFFER_HEIGHT		640

#define _DEPTH_BUFFER_WIDTH		(FRAME_BUFFER_WIDTH * 4)
#define _DEPTH_BUFFER_HEIGHT	(FRAME_BUFFER_HEIGHT * 4)

#define DELTA_X					(1.0f / _DEPTH_BUFFER_WIDTH)
#define DELTA_Y					(1.0f / _DEPTH_BUFFER_HEIGHT)

#define MAX_DEPTH_TEXTURES		MAX_LIGHTS

//#define _WITH_PCF_FILTERING
Texture2D<float> gtxtDepthTextures[MAX_DEPTH_TEXTURES] : register(t20);
SamplerComparisonState gssComparisonPCFShadow : register(s2);

struct LIGHT
{
	float4					m_cAmbient;
	float4					m_cDiffuse;
	float4					m_cSpecular;
	float3					m_vPosition;
	float 					m_fFalloff;
	float3					m_vDirection;
	float 					m_fTheta; //cos(m_fTheta)
	float3					m_vAttenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int 					m_nType;
	float					m_fRange;
	float					padding;
};

cbuffer cbLights : register(b4)
{
	LIGHT					gLights[MAX_LIGHTS];
	float4					gcGlobalAmbientLight;
	int						gnLights;
};

float Compute3x3ShadowFactor(float2 uv, float fDepth, uint nIndex)
{
	float fPercentLit = gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv, fDepth).r;
	/*fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, 0.0f), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, 0.0f), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(0.0f, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(0.0f, +DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(-DELTA_X, +DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, -DELTA_Y), fDepth).r;
	fPercentLit += gtxtDepthTextures[nIndex].SampleCmpLevelZero(gssComparisonPCFShadow, uv + float2(+DELTA_X, +DELTA_Y), fDepth).r;*/

	return(fPercentLit / 9.0f);
}

float4 DirectionalLight(int nIndex, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = -gLights[nIndex].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	float fSpecularFactor = 0.0f;
	if (fDiffuseFactor > 0.0f)
	{
		if (gMaterial.m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
		}
	}

	return((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse ) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular) * 5.0f );
}

float4 PointLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 SpotLight(int nIndex, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	float3 vToLight = gLights[nIndex].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	if (fDistance <= gLights[nIndex].m_fRange)
	{
		float fSpecularFactor = 0.0f;
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		if (fDiffuseFactor > 0.0f)
		{
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			}
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[nIndex].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[nIndex].m_fPhi) / (gLights[nIndex].m_fTheta - gLights[nIndex].m_fPhi)), 0.0f), gLights[nIndex].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[nIndex].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));

		return(((gLights[nIndex].m_cAmbient * gMaterial.m_cAmbient) + (gLights[nIndex].m_cDiffuse * fDiffuseFactor * gMaterial.m_cDiffuse) + (gLights[nIndex].m_cSpecular * fSpecularFactor * gMaterial.m_cSpecular)) * fAttenuationFactor * fSpotFactor);
	}
	return(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll(MAX_LIGHTS)] for (int i = 0; i < gnLights; i++)
	{
		if (gLights[i].m_bEnable)
		{
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				cColor += DirectionalLight(i, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera);
			}
		}
	}
	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;

	return(cColor);
}

float4 TerrainLighting(float3 vPosition, float3 vNormal, float4 uvs[MAX_LIGHTS])
{
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(vCameraPosition - vPosition);

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);


	[unroll(MAX_LIGHTS)] for (int i = 0; i < gnLights; i++)
	{
		if (gLights[i].m_bEnable)
		{
			float fShadowFactor = 1.0f;

			fShadowFactor = Compute3x3ShadowFactor(uvs[i].xy / uvs[i].ww, uvs[i].z / uvs[i].w, i);
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				//cColor += DirectionalLight(i, vNormal, vToCamera);
				cColor += DirectionalLight(i, vNormal, vToCamera) * fShadowFactor;
			}
			else if (gLights[i].m_nType == POINT_LIGHT)
			{
				cColor += PointLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				cColor += SpotLight(i, vPosition, vNormal, vToCamera) * fShadowFactor;
			}
		}
	}
	cColor += (gcGlobalAmbientLight * gMaterial.m_cAmbient);
	cColor.a = gMaterial.m_cDiffuse.a;

	return(cColor);
}

