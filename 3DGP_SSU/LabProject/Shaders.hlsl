#define FRAME_BUFFER_WIDTH 800.0f
#define FRAME_BUFFER_HEIGHT 600.0f

#include "Light.hlsl"
//정점 조명을 사용
#define _WITH_VERTEX_LIGHTING

//플레이어 객체의 데이터를 위한 상수 버퍼
cbuffer cbPlayerInfo : register(b0)
{
	matrix gmtxPlayerWorld : packoffset(c0);
};
//카메라 객체의 데이터를 위한 상수 버퍼(스펙큘러 조명 계산을 위하여 카메라의 위치 벡터를 추가)
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
	float3 gvCameraPosition : packoffset(c8);
};
//게임 객체의 데이터를 위한 상수 버퍼(게임 객체에 대한 재질 번호를 추가)
cbuffer cbGameObjectInfo : register(b2)
{
	matrix gmtxGameObject : packoffset(c0);
	uint gnMaterial : packoffset(c4);
};

////정점 셰이더의 입력을 위한 구조체를 선언한다. 
//struct VS_INPUT
//{
//	float3 position : POSITION;
//	float4 color : COLOR;
//};
//
////정점 셰이더의 출력(픽셀 셰이더의 입력)을 위한 구조체를 선언한다. 
//struct VS_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float4 color : COLOR;
//};
//
////정점 셰이더를 정의한다. 
//VS_OUTPUT VSDiffused(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	//정점을 변환(월드 변환, 카메라 변환, 투영 변환)한다. 
//	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), 
//	gmtxProjection);
//	output.color = input.color;
//
//	return(output);
//}
//
////픽셀 셰이더를 정의한다. 
//float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
//{
//	return(input.color);
//}

////정점 데이터와 인스턴싱 데이터를 위한 구조체이다. 
//struct VS_INSTANCING_INPUT
//{
//	float3 position : POSITION;
//	float4 color : COLOR;
//	float4x4 mtxTransform : WORLDMATRIX;
//	float4 instanceColor : INSTANCECOLOR;
//};
//
//struct VS_INSTANCING_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float4 color : COLOR;
//};
//
//VS_INSTANCING_OUTPUT VSInstancing(VS_INSTANCING_INPUT input)
//{
//	VS_INSTANCING_OUTPUT output;
//
//	output.position = mul(mul(mul(float4(input.position, 1.0f), input.mtxTransform),
//		gmtxView), gmtxProjection);
//	
//	//input.color + 
//	output.color = input.instanceColor;
//
//	return(output);
//}
//
//float4 PSInstancing(VS_INSTANCING_OUTPUT input) : SV_TARGET
//{
//	return(input.color);
//}

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

//정점 쉐이더의 입력 정점 구조
struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

//정점 쉐이더의 출력 정점 구조
struct VS_LIGHTING_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
#ifdef _WITH_VERTEX_LIGHTING
	float4 color : COLOR;
#else
	float3 normalW : NORMAL;
#endif
};

//정점 쉐이더 함수
VS_LIGHTING_OUTPUT VSLighting(VS_LIGHTING_INPUT input)
{
	VS_LIGHTING_OUTPUT output;
	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxGameObject);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	float3 normalW = mul(input.normal, (float3x3)gmtxGameObject);
#ifdef _WITH_VERTEX_LIGHTING
	output.color = Lighting(output.positionW, normalize(normalW));
#else
	output.normalW = normalW;
#endif
	return(output);
}

//픽셀 쉐이더 함수
float4 PSLighting(VS_LIGHTING_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_VERTEX_LIGHTING
	return(input.color);
#else
	float3 normalW = normalize(input.normalW);
	float4 color = Lighting(input.positionW, normalW);
	return(color);
#endif
}