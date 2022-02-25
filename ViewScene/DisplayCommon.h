/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#ifndef _DISPLAY_COMMON_H
#define _DISPLAY_COMMON_H

#include <fbxsdk.h>

char *ReplaceBlank(const char *value, const char chReplace);

void DisplayMetaDataConnections(FbxObject *pfbxNode);
void DisplayString(const char *pHeader, const char *value  = "", const char *pSuffix  = "", int nTabIndents = 0);
void DisplayBool(const char *pHeader, bool value, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(int value);
void DisplayInt(const char *pHeader, int value, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(const char *pHeader, int value1, int value2, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(const char *pHeader, int value1, int value2, int value3, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayInt(const char *pHeader, int value1, int value2, int value3, int value4, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayDouble(double value);
void DisplayDouble(const char *pHeader, double value, const char *pSuffix  = "", int nTabIndents = 0);
void Display2DVector(FbxVector2 value);
void Display2DVector(const char *pHeader, FbxVector2 value, const char *pSuffix  = "", int nTabIndents = 0);
void Display3DVector(FbxVector4 value);
void Display3DVector(const char *pHeader, FbxVector4 value, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayColor(FbxColor value);
void DisplayColor(const char *pHeader, FbxColor value, const char *pSuffix  = "", int nTabIndents = 0);
void Display4DVector(FbxVector4 value);
void Display4DVector(const char *pHeader, FbxVector4 value, const char *pSuffix  = "", int nTabIndents = 0);
void DisplayMatrix(FbxAMatrix value);

extern void WriteStringToFile(char *pszBuffer);

#endif // #ifndef _DISPLAY_COMMON_H


