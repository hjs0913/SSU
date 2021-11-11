cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxGameObject : packoffset(c0);
	uint		gnObjectID : packoffset(c4.x);
	uint		gnMaterialID : packoffset(c4.y);
};

#include "Light.hlsl"

Texture2DArray gtxtTextureArray : register(t0);
SamplerState gssDefaultSamplerState : register(s0);

cbuffer cbDrawOptions : register(b5)
{
	int4 gvDrawOptions : packoffset(c0);
};

struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssDefaultSamplerState, uvw);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_TEXTURED_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_LIGHTING_OUTPUT VSTexturedLighting(VS_TEXTURED_LIGHTING_INPUT input)
{
	VS_TEXTURED_LIGHTING_OUTPUT output;

	output.normalW = mul(input.normal, (float3x3)gmtxGameObject);
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

float4 PSTexturedLighting(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	float4 cColor = gtxtTextureArray.Sample(gssDefaultSamplerState, uvw);
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cColor * cIllumination);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct PS_MULTIPLE_RENDER_TARGETS_OUTPUT
{
	float4 f4Scene : SV_TARGET0; //Swap Chain Back Buffer

	float4 f4Color : SV_TARGET1;
	float4 f4Normal : SV_TARGET2;
	float4 f4Texture : SV_TARGET3;
	float4 f4Illumination : SV_TARGET4;
	float nObjectID : SV_TARGET5;
	float fzDepth : SV_TARGET6;
};

PS_MULTIPLE_RENDER_TARGETS_OUTPUT PSTexturedLightingToMultipleRTs(VS_TEXTURED_LIGHTING_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID)
{
	PS_MULTIPLE_RENDER_TARGETS_OUTPUT output;

	float3 uvw = float3(input.uv, nPrimitiveID / 2);
	output.f4Texture = gtxtTextureArray.Sample(gssDefaultSamplerState, uvw);

	input.normalW = normalize(input.normalW);
	output.f4Illumination = Lighting(input.positionW, input.normalW);

	output.f4Scene = output.f4Color = output.f4Illumination * output.f4Texture;

	output.f4Normal = float4(input.normalW.xyz * 0.5f + 0.5f, input.position.z);

	output.nObjectID = (float)gnObjectID;

//	output.zDepth = 1.0f - (input.position.z != 0.0f) ? (1.0f / input.position.z) : 1.0f;
//	output.zDepth = abs(1.0f - input.position.z);
	output.fzDepth = 1.0f - input.position.z;

//	output.fDepth = 1.0f - input.position.z;
//	output.fDepth = input.position.z;

	return(output);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
float4 VSPostProcessing(uint nVertexID : SV_VertexID) : SV_POSITION
{
	if (nVertexID == 0) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1) return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));

	if (nVertexID == 3) return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4) return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5) return(float4(-1.0f, -1.0f, 0.0f, 1.0f));

	return(float4(0, 0, 0, 0));
}

float4 PSPostProcessing(float4 position : SV_POSITION) : SV_Target
{
	return(float4(0.0f, 0.0f, 0.0f, 1.0f));
}

///////////////////////////////////////////////////////////////////////////////
//
struct VS_SCREEN_RECT_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_SCREEN_RECT_TEXTURED_OUTPUT VSScreenRectSamplingTextured(uint nVertexID : SV_VertexID)
{
	VS_SCREEN_RECT_TEXTURED_OUTPUT output = (VS_TEXTURED_OUTPUT)0;

	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	else if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }

	else if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	else if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	else if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

Texture2D gtxtInputTextures[7] : register(t1); //Color, Normal, Texture, Illumination, ObjectID, zDepth, Depth 

float4 GetColorFromDepth(float fDepth)
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (fDepth > 1.0f) cColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.00625f) cColor = float4(1.0f, 0.0f, 0.0f, 1.0f);
	else if (fDepth < 0.0125f) cColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.025f) cColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.05f) cColor = float4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (fDepth < 0.075f) cColor = float4(0.0f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.1f) cColor = float4(1.0f, 0.5f, 0.5f, 1.0f);
	else if (fDepth < 0.4f) cColor = float4(0.5f, 1.0f, 1.0f, 1.0f);
	else if (fDepth < 0.6f) cColor = float4(1.0f, 0.0f, 1.0f, 1.0f);
	else if (fDepth < 0.8f) cColor = float4(0.5f, 0.5f, 1.0f, 1.0f);
	else if (fDepth < 0.9f) cColor = float4(0.5f, 1.0f, 0.5f, 1.0f);
	else cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return(cColor);
}

static float gfLaplacians[9] = { -1.0f, -1.0f, -1.0f, -1.0f, 8.0f, -1.0f, -1.0f, -1.0f, -1.0f };
static int2 gnOffsets[9] = { { -1,-1 }, { 0,-1 }, { 1,-1 }, { -1,0 }, { 0,0 }, { 1,0 }, { -1,1 }, { 0,1 }, { 1,1 } };

float4 LaplacianEdge(float4 position)
{
	float fObjectEdgeness = 0.0f;
	float fNormalEdgeness = 0.0f;
	float3 cEdgeness = float3(0.0f, 0.0f, 0.0f);
	if ((uint(position.x) >= 1) || (uint(position.y) >= 1) || (uint(position.x) <= gtxtInputTextures[0].Length.x - 2) || (uint(position.y) <= gtxtInputTextures[0].Length.y - 2))
	{
		float fObjectID = gtxtInputTextures[4][int2(position.xy)].r;
		for (int i = 0; i < 9; i++)
		{
			if (fObjectID != gtxtInputTextures[4][int2(position.xy) + gnOffsets[i]].r) fObjectEdgeness = 1.0f;

			float3 vNormal = gtxtInputTextures[1][int2(position.xy) + gnOffsets[i]].xyz * 2.0f - 1.0f;
			cEdgeness += gfLaplacians[i] * vNormal;
		}
		fNormalEdgeness = cEdgeness.r * 0.3f + cEdgeness.g * 0.59f + cEdgeness.b * 0.11f;
	}
	float3 cColor = gtxtInputTextures[0][int2(position.xy)].rgb;

	if (fObjectEdgeness == 1.0f) 
		cColor = float3(1.0f, 0.0f, 0.0f);
	else
		cColor += fNormalEdgeness;

	return(float4(cColor, 1.0f));
}

float4 PSScreenRectSamplingTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	switch (gvDrawOptions.x)
	{
		case 84: //'T'
		{
			cColor = gtxtInputTextures[2].Sample(gssDefaultSamplerState, input.uv);
			break;
		}
		case 76: //'L'
		{
			cColor = gtxtInputTextures[3].Sample(gssDefaultSamplerState, input.uv);
			break;
		}
		case 78: //'N'
		{
			cColor = gtxtInputTextures[1].Sample(gssDefaultSamplerState, input.uv);
			break;
		}
		case 68: //'D'
		{
			float fDepth = gtxtInputTextures[6].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r;
			cColor = GetColorFromDepth(1.0f - fDepth);
			break;
		}
		case 90: //'Z' 
		{
			float fDepth = gtxtInputTextures[5].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r;
			cColor = GetColorFromDepth(fDepth);
			break;
		}
		case 79: //'O'
		{
			uint fObjectID = (uint)gtxtInputTextures[4].Load(uint3((uint)input.position.x, (uint)input.position.y, 0)).r;
//			uint fObjectID = (uint)gtxtInputTextures[4][int2(input.position.xy)].r;
			if (fObjectID == 0) cColor.rgb = float3(1.0f, 1.0f, 1.0f);
			else if (fObjectID <= 1000) cColor.rgb = float3(1.0f, 0.0f, 0.0f);
			else if (fObjectID <= 2000) cColor.rgb = float3(0.0f, 1.0f, 0.0f);
			else if (fObjectID <= 3000) cColor.rgb = float3(0.0f, 0.0f, 1.0f);
			else if (fObjectID <= 4000) cColor.rgb = float3(0.0f, 1.0f, 1.0f);
			else if (fObjectID <= 5000) cColor.rgb = float3(1.0f, 1.0f, 0.0f);
			else if (fObjectID <= 6000) cColor.rgb = float3(1.0f, 1.0f, 1.0f);
			else if (fObjectID <= 7000) cColor.rgb = float3(1.0f, 0.5f, 0.5f);
			else cColor.rgb = float3(0.3f, 0.75f, 0.5f);

//			cColor.rgb = fObjectID;
			break;
		}
		case 69: //'E'
		{
			cColor = LaplacianEdge(input.position);
			break;
		}
	}

	return(cColor);
}
