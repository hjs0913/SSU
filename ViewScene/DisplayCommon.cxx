/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include "DisplayCommon.h"
#if defined (FBXSDK_ENV_MAC)
// disable the “format not a string literal and no format arguments?warning since
// the PrintToFile calls made here are all valid calls and there is no secuity risk
#pragma GCC diagnostic ignored "-Wformat-security"
#endif

void DisplayMetaDataConnections(FbxObject *pfbxObject)
{
	int nbMetaData = pfbxObject->GetSrcObjectCount<FbxObjectMetaData>();
    if (nbMetaData > 0)
        DisplayString("    MetaData connections ");

    for (int i = 0; i < nbMetaData; i++)
    {
        FbxObjectMetaData* metaData = pfbxObject->GetSrcObject<FbxObjectMetaData>(i);
        DisplayString("        Name: ", (char*)metaData->GetName());
    }
}

FbxString gfbxString;
char *ReplaceBlank(const char *value, const char chReplace)
{
	gfbxString = value;
	gfbxString.ReplaceAll(' ', chReplace);
	return(gfbxString.Buffer());
}

void DisplayString(const char* pHeader, const char* value /* = "" */, const char* pSuffix /* = "" */, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
    fbxString += value;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayBool(const char* pHeader, bool value, const char* pSuffix /* = "" */, int nTabIndents)
{
    FbxString fbxString;

    fbxString = pHeader;
    fbxString += value ? "true" : "false";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(int value)
{
	FbxString fbxString;

	fbxString += value;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(const char* pHeader, int value, const char* pSuffix /* = "" */, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

    fbxString = pHeader;
    fbxString += value;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(const char* pHeader, int value1, int value2, const char* pSuffix /* = "" */, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(const char* pHeader, int value1, int value2, int value3, const char* pSuffix /* = "" */, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayInt(const char* pHeader, int value1, int value2, int value3, int value4, const char* pSuffix /* = "" */, int nTabIndents)
{
	for (int i = 0; i < nTabIndents; i++) WriteStringToFile("\t");

	FbxString fbxString;

	fbxString = pHeader;
	fbxString += value1;
	fbxString += " ";
	fbxString += value2;
	fbxString += " ";
	fbxString += value3;
	fbxString += " ";
	fbxString += value4;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayDouble(double value)
{
	FbxString fbxString;

	fbxString += value;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayDouble(const char* pHeader, double value, const char* pSuffix /* = "" */, int nTabIndents)
{
    FbxString fbxString;
    FbxString lFloatValue = (float) value;

    lFloatValue = value <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
    lFloatValue = value >=  HUGE_VAL ?  "INFINITY" : lFloatValue.Buffer();

    fbxString = pHeader;
    fbxString += lFloatValue;
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display2DVector(FbxVector2 value)
{
	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];

	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display2DVector(const char* pHeader, FbxVector2 value, const char* pSuffix  /* = "" */, int nTabIndents)
{
	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display3DVector(FbxVector4 value)
{
	FbxString fbxString;

	fbxString += (float)value[0];
	fbxString += " ";
	fbxString += (float)value[1];
	fbxString += " ";
	fbxString += (float)value[2];
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display3DVector(const char* pHeader, FbxVector4 value, const char* pSuffix /* = "" */, int nTabIndents)
{
	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];
	FbxString lFloatValue3 = (float)value[2];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = value[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = value[2] >=  HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += " ";
	fbxString += lFloatValue2;
	fbxString += " ";
	fbxString += lFloatValue3;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void Display4DVector(FbxVector4 value)
{
	FbxString fbxString;

	fbxString += (float)value[0];
	fbxString += " ";
	fbxString += (float)value[1];
	fbxString += " ";
	fbxString += (float)value[2];
	fbxString += " ";
	fbxString += (float)value[3];
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void Display4DVector(const char* pHeader, FbxVector4 value, const char* pSuffix /* = "" */, int nTabIndents)
{
	FbxString fbxString;
	FbxString lFloatValue1 = (float)value[0];
	FbxString lFloatValue2 = (float)value[1];
	FbxString lFloatValue3 = (float)value[2];
	FbxString lFloatValue4 = (float)value[3];

	lFloatValue1 = value[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
	lFloatValue1 = value[0] >=  HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
	lFloatValue2 = value[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
	lFloatValue2 = value[1] >=  HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
	lFloatValue3 = value[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
	lFloatValue3 = value[2] >=  HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
	lFloatValue4 = value[3] <= -HUGE_VAL ? "-INFINITY" : lFloatValue4.Buffer();
	lFloatValue4 = value[3] >=  HUGE_VAL ? "INFINITY" : lFloatValue4.Buffer();

	fbxString = pHeader;
	fbxString += lFloatValue1;
	fbxString += ", ";
	fbxString += lFloatValue2;
	fbxString += ", ";
	fbxString += lFloatValue3;
	fbxString += ", ";
	fbxString += lFloatValue4;
	fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayMatrix(FbxAMatrix value)
{
	Display4DVector(value.GetRow(0));
	Display4DVector(value.GetRow(1));
	Display4DVector(value.GetRow(2));
	Display4DVector(value.GetRow(3));
}

void DisplayColor(const char* pHeader, FbxPropertyT<FbxDouble3> value, const char* pSuffix /* = "" */, int nTabIndents)
{
    FbxString fbxString;

    fbxString = pHeader;
    //fbxString += (float) value.mRed;
    //fbxString += (double)value.GetArrayItem(0);
    fbxString += "(R), ";
    //fbxString += (float) value.mGreen;
    //fbxString += (double)value.GetArrayItem(1);
    fbxString += "(G), ";
    //fbxString += (float) value.mBlue;
    //fbxString += (double)value.GetArrayItem(2);
    fbxString += "(B)";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

void DisplayColor(FbxColor value)
{
	FbxString fbxString;

	fbxString += (float)value.mRed;
	fbxString += (float)value.mGreen;
	fbxString += (float)value.mBlue;
	fbxString += " ";
	WriteStringToFile(fbxString.Buffer());
}

void DisplayColor(const char* pHeader, FbxColor value, const char* pSuffix /* = "" */, int nTabIndents)
{
    FbxString fbxString;

    fbxString = pHeader;
    fbxString += (float)value.mRed;
    fbxString += "(R), ";
    fbxString += (float)value.mGreen;
    fbxString += "(G), ";
    fbxString += (float)value.mBlue;
    fbxString += "(B)";
    fbxString += pSuffix;
	WriteStringToFile(fbxString.Buffer());
}

