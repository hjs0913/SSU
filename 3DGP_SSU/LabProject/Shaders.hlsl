#define FRAME_BUFFER_WIDTH 800.0f
#define FRAME_BUFFER_HEIGHT 600.0f

#include "Light.hlsl"
//���� ������ ���
#define _WITH_VERTEX_LIGHTING

//�÷��̾� ��ü�� �����͸� ���� ��� ����
cbuffer cbPlayerInfo : register(b0)
{
	matrix gmtxPlayerWorld : packoffset(c0);
};
//ī�޶� ��ü�� �����͸� ���� ��� ����(����ŧ�� ���� ����� ���Ͽ� ī�޶��� ��ġ ���͸� �߰�)
cbuffer cbCameraInfo : register(b1)
{
	matrix gmtxView : packoffset(c0);
	matrix gmtxProjection : packoffset(c4);
	float3 gvCameraPosition : packoffset(c8);
};
//���� ��ü�� �����͸� ���� ��� ����(���� ��ü�� ���� ���� ��ȣ�� �߰�)
cbuffer cbGameObjectInfo : register(b2)
{
	matrix gmtxGameObject : packoffset(c0);
	uint gnMaterial : packoffset(c4);
};

////���� ���̴��� �Է��� ���� ����ü�� �����Ѵ�. 
//struct VS_INPUT
//{
//	float3 position : POSITION;
//	float4 color : COLOR;
//};
//
////���� ���̴��� ���(�ȼ� ���̴��� �Է�)�� ���� ����ü�� �����Ѵ�. 
//struct VS_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float4 color : COLOR;
//};
//
////���� ���̴��� �����Ѵ�. 
//VS_OUTPUT VSDiffused(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	//������ ��ȯ(���� ��ȯ, ī�޶� ��ȯ, ���� ��ȯ)�Ѵ�. 
//	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), 
//	gmtxProjection);
//	output.color = input.color;
//
//	return(output);
//}
//
////�ȼ� ���̴��� �����Ѵ�. 
//float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
//{
//	return(input.color);
//}

////���� �����Ϳ� �ν��Ͻ� �����͸� ���� ����ü�̴�. 
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

//���� ���̴��� �Է� ���� ����
struct VS_LIGHTING_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

//���� ���̴��� ��� ���� ����
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

//���� ���̴� �Լ�
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

//�ȼ� ���̴� �Լ�
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