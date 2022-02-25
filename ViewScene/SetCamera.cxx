/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This file contains functions to:
// 1) find the current camera;
// 2) get the relevant settings of a camera depending on it's projection
//    type and aperture mode;
// 3) compute the orientation of a camera.
//
/////////////////////////////////////////////////////////////////////////

#include <fbxsdk.h>

#include "GlFunctions.h"
#include "SetCamera.h"
#include "SceneContext.h"

#define HFOV2VFOV(h, ar) (2.0 * atan((ar) * tan( (h * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectY / aspectX
#define VFOV2HFOV(v, ar) (2.0 * atan((ar) * tan( (v * FBXSDK_PI_DIV_180) * 0.5)) * FBXSDK_180_DIV_PI) //ar : aspectX / aspectY

FbxCamera* GetCurrentCamera(FbxScene *pfbxScene, FbxTime& rfbxTime, FbxAnimLayer* pfbxAnimLayer);
void GetCameraAnimatedParameters(FbxNode *pfbxNode, FbxTime& rfbxTime, FbxAnimLayer* pfbxAnimLayer);
bool IsProducerCamera(FbxScene *pfbxScene, FbxCamera* pCamera);

static double gsOrthoCameraScale = 178.0; 

void SetCamera(FbxScene *pfbxScene, FbxTime& rfbxTime, FbxAnimLayer* pfbxAnimLayer, int nWindowWidth, int nWindowHeight)
{
    FbxCamera *pfbxCamera = GetCurrentCamera(pfbxScene, rfbxTime, pfbxAnimLayer);
    if (pfbxCamera == NULL) return;
    FbxNode *pfbxCameraNode = (pfbxCamera) ? pfbxCamera->GetNode() : NULL;

    FbxVector4 lEye(0,0,1);
    FbxVector4 lCenter(0,0,0);
    FbxVector4 lUp(0,1,0);
    FbxVector4 lForward, lRight;

    if (pfbxCamera)
    {
        lEye = pfbxCamera->Position.Get();
        lUp = pfbxCamera->UpVector.Get();
    }

    if (pfbxCameraNode && pfbxCameraNode->GetTarget())
    {
		lCenter = pfbxCameraNode->GetTarget()->EvaluateGlobalTransform(rfbxTime).GetT();
        //lCenter = GetGlobalTransform(pfbxCameraNode->GetTarget(), rfbxTime).GetT();
    }
    else
    {
        if (!pfbxCameraNode || IsProducerCamera(pfbxScene, pfbxCamera))
        {
            if (pfbxCamera) lCenter = pfbxCamera->InterestPosition.Get();
        }
        else
        {
            FbxAMatrix lGlobalRotation;
            FbxVector4 lRotationVector(pfbxCameraNode->EvaluateGlobalTransform(rfbxTime).GetR());
            lGlobalRotation.SetR(lRotationVector);

            FbxVector4 lInterestPosition(pfbxCamera->InterestPosition.Get());
            FbxVector4 lCameraGlobalPosition(pfbxCameraNode->EvaluateGlobalTransform(rfbxTime).GetT());
            double lLength = (FbxVector4(lInterestPosition - lCameraGlobalPosition).Length());

            lRotationVector = FbxVector4(1.0,0,0);
            lCenter = lGlobalRotation.MultT(lRotationVector);
            lCenter *= lLength;
            lCenter += lEye;

            lRotationVector = FbxVector4(0,1.0,0);
            lUp = lGlobalRotation.MultT(lRotationVector);
        }
    }

    lForward = lCenter - lEye;
    lForward.Normalize();
    lRight = lForward.CrossProduct(lUp);
    lRight.Normalize();
    lUp = lRight.CrossProduct(lForward);
    lUp.Normalize();

    double lRadians = 0;

    if (pfbxCamera) lRadians = pfbxCamera->Roll.Get() * FBXSDK_PI_DIV_180;
    lUp = lUp * cos(lRadians) + lRight * sin(lRadians);
   
    double lNearPlane = 0.01;
    if (pfbxCamera) lNearPlane = pfbxCamera->GetNearPlane();    
    double lFarPlane = 4000.0;
    if (pfbxCamera) lFarPlane = pfbxCamera->GetFarPlane();

    FbxVector4 lCameraScaling = pfbxCameraNode->EvaluateGlobalTransform(rfbxTime).GetS();
    static const int  FORWARD_SCALE = 2;
    
    lNearPlane *= lCameraScaling[ FORWARD_SCALE];
    lFarPlane *= lCameraScaling[ FORWARD_SCALE];

	if (pfbxCamera && pfbxCamera->ProjectionType.Get() == FbxCamera::ePerspective)
    {
        FbxCamera::EAspectRatioMode lCamAspectRatioMode = pfbxCamera->GetAspectRatioMode();
        double lAspectX = pfbxCamera->AspectWidth.Get();
        double lAspectY = pfbxCamera->AspectHeight.Get();
        double lAspectRatio = 1.333333;
        switch (lCamAspectRatioMode)
        {
        case FbxCamera::eWindowSize:
            lAspectRatio = lAspectX / lAspectY;
            break;
        case FbxCamera::eFixedRatio:
            lAspectRatio = lAspectX;

            break;
        case FbxCamera::eFixedResolution:
            lAspectRatio = lAspectX / lAspectY * pfbxCamera->GetPixelRatio();
            break;
        case FbxCamera::eFixedWidth:
            lAspectRatio = pfbxCamera->GetPixelRatio() / lAspectY;
            break;
        case FbxCamera::eFixedHeight:
            lAspectRatio = pfbxCamera->GetPixelRatio() * lAspectX;
            break;
        default:
            break;
        }

        double lFilmHeight = pfbxCamera->GetApertureHeight();
        double lFilmWidth = pfbxCamera->GetApertureWidth() * pfbxCamera->GetSqueezeRatio();
        double lApertureRatio = lFilmHeight / lFilmWidth;

        lAspectRatio = 1 / lAspectRatio;
        FbxCamera::EGateFit lCameraGateFit = pfbxCamera->GateFit.Get();
        switch (lCameraGateFit)
        {
        case FbxCamera::eFitFill:
            if (lApertureRatio > lAspectRatio)  // the same as eHORIZONTAL_FIT
            {
                lFilmHeight = lFilmWidth * lAspectRatio;
                pfbxCamera->SetApertureHeight(lFilmHeight);
                lApertureRatio = lFilmHeight / lFilmWidth;
            }
            else if (lApertureRatio < lAspectRatio) //the same as eVERTICAL_FIT
            {
                lFilmWidth = lFilmHeight / lAspectRatio;
                pfbxCamera->SetApertureWidth(lFilmWidth);
                lApertureRatio = lFilmHeight / lFilmWidth;
            }
            break;
        case FbxCamera::eFitVertical:
            lFilmWidth = lFilmHeight / lAspectRatio;
            pfbxCamera->SetApertureWidth(lFilmWidth);
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;
        case FbxCamera::eFitHorizontal:
            lFilmHeight = lFilmWidth * lAspectRatio;
            pfbxCamera->SetApertureHeight(lFilmHeight);
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;
        case FbxCamera::eFitStretch:
            lAspectRatio = lApertureRatio;
            break;
        case FbxCamera::eFitOverscan:
            if( lFilmWidth > lFilmHeight)
            {
                lFilmHeight = lFilmWidth * lAspectRatio;
            }
            else
            {
                lFilmWidth = lFilmHeight / lAspectRatio;
            }
            lApertureRatio = lFilmHeight / lFilmWidth;
            break;
        case FbxCamera::eFitNone:
			break;
        default:
            break;
        }
        lAspectRatio = 1 / lAspectRatio;

        double lFieldOfViewX = 0.0;
        double lFieldOfViewY = 0.0;
        if (pfbxCamera->GetApertureMode() == FbxCamera::eVertical)
        {
            lFieldOfViewY = pfbxCamera->FieldOfView.Get();
            lFieldOfViewX = VFOV2HFOV( lFieldOfViewY, 1 / lApertureRatio);
        }
        else if (pfbxCamera->GetApertureMode() == FbxCamera::eHorizontal)
        {
            lFieldOfViewX = pfbxCamera->FieldOfView.Get(); 
            lFieldOfViewY = HFOV2VFOV( lFieldOfViewX, lApertureRatio);
        }
        else if (pfbxCamera->GetApertureMode() == FbxCamera::eFocalLength)
        {
            lFieldOfViewX = pfbxCamera->ComputeFieldOfView(pfbxCamera->FocalLength.Get());    
            lFieldOfViewY = HFOV2VFOV( lFieldOfViewX, lApertureRatio);
        }
        else if (pfbxCamera->GetApertureMode() == FbxCamera::eHorizAndVert) 
		{
            lFieldOfViewX = pfbxCamera->FieldOfViewX.Get();
            lFieldOfViewY = pfbxCamera->FieldOfViewY.Get();
        }

        double lRealScreenRatio = (double)nWindowWidth / (double)nWindowHeight;
        int lViewPortPosX = 0, lViewPortPosY = 0, lViewPortSizeX = nWindowWidth, lViewPortSizeY = nWindowHeight;
        if (lRealScreenRatio > lAspectRatio)
        {
            lViewPortSizeY = nWindowHeight;
            lViewPortSizeX = (int)(lViewPortSizeY * lAspectRatio);
            lViewPortPosY = 0;
            lViewPortPosX = (int)((nWindowWidth - lViewPortSizeX) * 0.5);
        }
        else
        {
            lViewPortSizeX = nWindowWidth;
            lViewPortSizeY = (int)(lViewPortSizeX / lAspectRatio);
            lViewPortPosX = 0;
            lViewPortPosY = (int)((nWindowHeight - lViewPortSizeY) * 0.5);
        }

        double lFilmOffsetX = pfbxCamera->FilmOffsetX.Get();
        double lFilmOffsetY = pfbxCamera->FilmOffsetY.Get();
        lFilmOffsetX = 0 - lFilmOffsetX / lFilmWidth * 2.0;
        lFilmOffsetY = 0 - lFilmOffsetY / lFilmHeight * 2.0;

        GlSetCameraPerspective(lFieldOfViewY, lAspectRatio, lNearPlane, lFarPlane, lEye, lCenter, lUp, lFilmOffsetX, lFilmOffsetY);       

        glViewport(lViewPortPosX, lViewPortPosY, lViewPortSizeX, lViewPortSizeY);       
    }
    else
    {
        double lPixelRatio = 1.0;
        if (pfbxCamera) lPixelRatio = pfbxCamera->GetPixelRatio();  

        double lLeftPlane, lRightPlane, lBottomPlane, lTopPlane;

        if (nWindowWidth < nWindowHeight) 
        {   
            lLeftPlane   = -gsOrthoCameraScale * lPixelRatio;
            lRightPlane  =  gsOrthoCameraScale * lPixelRatio;
            lBottomPlane = -gsOrthoCameraScale * nWindowHeight / nWindowWidth;
            lTopPlane    =  gsOrthoCameraScale * nWindowHeight / nWindowWidth;
        } 
        else 
        {
            nWindowWidth *= (int) lPixelRatio;
            lLeftPlane   = -gsOrthoCameraScale * nWindowWidth / nWindowHeight;
            lRightPlane  =  gsOrthoCameraScale * nWindowWidth / nWindowHeight;
            lBottomPlane = -gsOrthoCameraScale;
            lTopPlane    =  gsOrthoCameraScale;
        }

        GlSetCameraOrthogonal(lLeftPlane, lRightPlane, lBottomPlane, lTopPlane, lNearPlane, lFarPlane, lEye, lCenter, lUp);
    }
}

FbxCamera* GetCurrentCamera(FbxScene *pfbxScene, FbxTime& rfbxTime, FbxAnimLayer* pfbxAnimLayer)
{
    FbxGlobalSettings& lGlobalSettings = pfbxScene->GetGlobalSettings();
    FbxGlobalCameraSettings& lGlobalCameraSettings = pfbxScene->GlobalCameraSettings();
    FbxString lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

    if (lGlobalCameraSettings.GetCameraProducerPerspective() == NULL && lGlobalCameraSettings.GetCameraProducerBottom() == NULL && lGlobalCameraSettings.GetCameraProducerTop() == NULL && lGlobalCameraSettings.GetCameraProducerFront() == NULL && lGlobalCameraSettings.GetCameraProducerBack() == NULL && lGlobalCameraSettings.GetCameraProducerRight() == NULL && lGlobalCameraSettings.GetCameraProducerLeft() == NULL)
    {
        lGlobalCameraSettings.CreateProducerCameras();
    }

    if (lCurrentCameraName.Compare(FBXSDK_CAMERA_PERSPECTIVE) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerPerspective();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BOTTOM) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerBottom();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_TOP) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerTop();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_FRONT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerFront();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_BACK) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerBack();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_RIGHT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerRight();
    }
    else if (lCurrentCameraName.Compare(FBXSDK_CAMERA_LEFT) == 0)
    {
        return lGlobalCameraSettings.GetCameraProducerLeft();
    }    

    return lGlobalCameraSettings.GetCameraProducerPerspective();
}

void GetCameraAnimatedParameters(FbxNode *pfbxNode, FbxTime& rfbxTime, FbxAnimLayer* pfbxAnimLayer)
{
    FbxCamera* pfbxCamera = (FbxCamera*) pfbxNode->GetNodeAttribute();
    pfbxCamera->Position.Set(pfbxNode->EvaluateGlobalTransform(rfbxTime).GetT());

    FbxAnimCurve* fc = pfbxCamera->Roll.GetCurve(pfbxAnimLayer);
    if (fc) pfbxCamera->Roll.Set(fc->Evaluate(rfbxTime));

    FbxCamera::EApertureMode lCameraApertureMode = pfbxCamera->GetApertureMode();
    if (lCameraApertureMode == FbxCamera::eHorizontal || lCameraApertureMode == FbxCamera::eVertical) 
    {
        double lFieldOfView = pfbxCamera->FieldOfView.Get();
        fc = pfbxCamera->FieldOfView.GetCurve(pfbxAnimLayer);
        if (fc) lFieldOfView = fc->Evaluate(rfbxTime);

        pfbxCamera->FieldOfView.Set( lFieldOfView);
        pfbxCamera->FocalLength.Set( pfbxCamera->ComputeFocalLength( lFieldOfView));       
    }
    else if (lCameraApertureMode == FbxCamera::eHorizAndVert)
    {
        double lOldFieldOfViewX = pfbxCamera->FieldOfViewX.Get();
        double lOldFieldOfViewY = pfbxCamera->FieldOfViewY.Get();

        double lNewFieldOfViewX = lOldFieldOfViewX;
        double lNewFieldOfViewY = lOldFieldOfViewY;
        fc = pfbxCamera->FieldOfViewX.GetCurve(pfbxAnimLayer);
        if (fc) lNewFieldOfViewX = fc->Evaluate(rfbxTime);

        fc = pfbxCamera->FieldOfViewY.GetCurve(pfbxAnimLayer);
        if (fc) lNewFieldOfViewY = fc->Evaluate(rfbxTime);

        pfbxCamera->FieldOfViewX.Set(lNewFieldOfViewX);
        pfbxCamera->FieldOfViewY.Set(lNewFieldOfViewY);

        //update aspect
        double lUpdatedApertureX = pfbxCamera->GetApertureWidth();
        double lUpdatedApertureY = pfbxCamera->GetApertureHeight();
        lUpdatedApertureX *= tan( lNewFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180) / tan( lOldFieldOfViewX * 0.5 * FBXSDK_PI_DIV_180);
        lUpdatedApertureY *= tan( lNewFieldOfViewY * 0.5 * FBXSDK_PI_DIV_180) / tan( lOldFieldOfViewY * 0.5 * FBXSDK_PI_DIV_180);
        
        pfbxCamera->FilmWidth.Set( lUpdatedApertureX);
        pfbxCamera->FilmHeight.Set( lUpdatedApertureY);
        pfbxCamera->FilmAspectRatio.Set( lUpdatedApertureX / lUpdatedApertureY);


    }
    else if ( lCameraApertureMode == FbxCamera::eFocalLength)
    {
        double lFocalLength = pfbxCamera->FocalLength.Get();
        fc = pfbxCamera->FocalLength.GetCurve(pfbxAnimLayer);
        if (fc && fc ->Evaluate(rfbxTime))
            lFocalLength = fc->Evaluate( rfbxTime);
            

        //update FOV and focal length
        pfbxCamera->FocalLength.Set( lFocalLength);
        pfbxCamera->FieldOfView.Set( pfbxCamera->ComputeFieldOfView( lFocalLength));
    }
}

bool IsProducerCamera(FbxScene *pfbxScene, FbxCamera* pCamera)
{
    FbxGlobalCameraSettings& lGlobalCameraSettings = pfbxScene->GlobalCameraSettings();
    if (pCamera == lGlobalCameraSettings.GetCameraProducerPerspective())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerTop())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerBottom())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerFront())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerBack())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerRight())
        return(true);
    if (pCamera == lGlobalCameraSettings.GetCameraProducerLeft())
        return(true);

    return(false);
}

FbxCamera* GetCurrentCamera(FbxScene *pfbxScene)
{
    FbxCamera* lRet = NULL;
    FbxString     lCurrentCameraName;

    FbxGlobalCameraSettings& lGlobalCameraSettings = pfbxScene->GlobalCameraSettings();
    FbxGlobalSettings& lGlobalSettings = pfbxScene->GetGlobalSettings();

    lCurrentCameraName = lGlobalSettings.GetDefaultCamera();

    if (lCurrentCameraName == FBXSDK_CAMERA_PERSPECTIVE)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerPerspective();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_TOP)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerTop();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_BOTTOM)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerBottom();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_FRONT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerFront();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_BACK)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerBack();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_RIGHT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerRight();
    }
    else if (lCurrentCameraName == FBXSDK_CAMERA_LEFT)
    {
        lRet = lGlobalCameraSettings.GetCameraProducerLeft();
    }
    else
    {
        FbxNode * pfbxCameraNode = pfbxScene->FindNodeByName( lCurrentCameraName);
        if( pfbxCameraNode)
        {
            lRet = pfbxCameraNode->GetCamera();
        }
    }
    return lRet;
}


double TransformAperture( double pAperture, double rfbxTransform)
{
    double lTransformAperture = ( pAperture + rfbxTransform);
    if( lTransformAperture < 0.25)
    {
        lTransformAperture = 0.25;
    }
    if( lTransformAperture  > 179.0)
    {
        lTransformAperture = 179.0;
    }
    return lTransformAperture;
}

void UpdatePerspCameraAttributes( FbxCamera* pCamera, double pNewApertureW, double pNewApertureH)
{

	if( pCamera == NULL || FbxAnimUtilities::IsAnimated( pCamera))
        return;
    // update focal length or field of view.
    double lApertureWidthOrig = pCamera->GetApertureWidth();
    double lApertureHeightOrig = pCamera->GetApertureHeight();

    if (pCamera->GetApertureMode() == FbxCamera::eFocalLength) {
        // update focal length according to hypothetic new apertures.
        double lFLOrig  = pCamera->FocalLength.Get();
        double lFOVOrig = pCamera->ComputeFieldOfView(lFLOrig); // recompute to be sure.
        // transform aperture width and height temporarily
        pCamera->SetApertureWidth( pNewApertureW );
        pCamera->SetApertureHeight(pNewApertureH );
        double lFLNew  = pCamera->ComputeFocalLength(lFOVOrig);
        double lFOVNew = pCamera->ComputeFieldOfView(lFLNew);
        pCamera->FocalLength.Set(lFLNew);
        pCamera->FieldOfView.Set(lFOVNew);
    } else if (pCamera->GetApertureMode() == FbxCamera::eVertical
        || pCamera->GetApertureMode() == FbxCamera::eHorizontal) {
            double lFOVOrig = pCamera->FieldOfView.Get();
            double lFLOrig = pCamera->ComputeFocalLength(lFOVOrig); // recompute to be sure.
            // transform aperture width and height temporarily
            pCamera->SetApertureWidth(pNewApertureW );
            pCamera->SetApertureHeight(pNewApertureH );
            double lFOVNew = pCamera->ComputeFieldOfView(lFLOrig);
            double lFLNew = pCamera->ComputeFocalLength(lFOVNew);
            pCamera->FieldOfView.Set(lFOVNew);
            pCamera->FocalLength.Set(lFLNew);
    } else if (pCamera->GetApertureMode() == FbxCamera::eHorizAndVert) {
        double lFOVOrigX = pCamera->FieldOfViewX.Get();
        double lFLOrig = pCamera->ComputeFocalLength(lFOVOrigX); // recompute to be sure.
        // transform aperture width and height temporarily
        pCamera->SetApertureWidth(pNewApertureW );
        pCamera->SetApertureHeight(pNewApertureH );
        double lFOVNewX = pCamera->ComputeFieldOfView(lFLOrig);
        double lFOVNewY = pCamera->ComputeFieldOfView(lFLOrig);
        double lFLNew = pCamera->ComputeFocalLength(lFOVNewX);
        pCamera->FieldOfViewY.Set(lFOVNewX);
        pCamera->FieldOfViewY.Set(lFOVNewY);
        pCamera->FocalLength.Set(lFLNew);
    }
    // reset aperture width and height
    pCamera->SetApertureWidth(lApertureWidthOrig);
    pCamera->SetApertureHeight(lApertureHeightOrig);
}


void CameraZoom(FbxScene *pfbxScene, int nZoomDepth, int nZoomMode)
{
    FbxCamera *pfbxCamera = GetCurrentCamera(pfbxScene);
    if (pfbxCamera == NULL) return;
    if (nZoomMode == ZOOM_FOCAL_LENGTH)
    {
        if (pfbxCamera->ProjectionType.Get() == FbxCamera::ePerspective)
        {
            double fTransform = 0 - nZoomDepth / 400.0;
            double fApertureW = pfbxCamera->GetApertureWidth();
            fApertureW = TransformAperture(fApertureW, fTransform);
            double fApertureH = pfbxCamera->GetApertureHeight();
            fApertureH = TransformAperture(fApertureH, fTransform);
            UpdatePerspCameraAttributes( pfbxCamera, fApertureW, fApertureH);
        }
        else
        {
            if( nZoomDepth > 0)
                gsOrthoCameraScale *= 0.8;
            else
                gsOrthoCameraScale *= 1.25;
        }
    }
    else
    {
        FbxNode *   pfbxCameraNode = pfbxCamera ? pfbxCamera->GetNode() : NULL;

        // Compute the camera position and direction.
        FbxVector4 lEye(0,0,1);
        FbxVector4 lCenter(0,0,0);
        FbxVector4 lForward(0,0,0);

        if (pfbxCamera)
        {
            lEye = pfbxCamera->Position.Get();
        }

        if (pfbxCameraNode && pfbxCameraNode->GetTarget())
        {
            lCenter = pfbxCameraNode->GetTarget()->LclTranslation.Get();
            lForward = lCenter - lEye;
        }
        else
        {
            if (!pfbxCameraNode || IsProducerCamera(pfbxScene, pfbxCamera))
            {
                if (pfbxCamera)
                {
                    lCenter = pfbxCamera->InterestPosition.Get();
                    lForward = lCenter - lEye;
                }
            }
            else
            {
                // Get the direction
                FbxAMatrix lGlobalRotation;
                FbxVector4 lRotationVector( pfbxCameraNode->LclRotation.Get());
                lGlobalRotation.SetR(lRotationVector);

                // Set the center.
                // A camera with rotation = {0,0,0} points to the X direction. So create a
                // vector in the X direction, rotate that vector by the global rotation amount
                // and then position the center by scaling and translating the resulting vector
                lRotationVector = FbxVector4(1.0,0,0);
                lForward = lGlobalRotation.MultT(lRotationVector);
            }
        }
        lForward.Normalize();
        lEye += lForward * nZoomDepth;
        FbxDouble3 lPosition(lEye[0], lEye[1], lEye[2]);
        pfbxCamera->Position.Set(lPosition);
        
    }
}

void CameraOrbit(FbxScene *pfbxScene, FbxVector4 lOrigCamPos, double OrigRoll, int dX, int dY)
{
    // Orbit the camera horizontally dX degrees, vertically dY degrees.
    FbxCamera* pfbxCamera = GetCurrentCamera(pfbxScene);
    if (!pfbxCamera) return;
    FbxGlobalCameraSettings& lGlobalCameraSettings = pfbxScene->GlobalCameraSettings();
    if (pfbxCamera != lGlobalCameraSettings.GetCameraProducerPerspective()) return;
    if (pfbxCamera->LockMode.Get()) return;
    if (dX == 0 && dY == 0) return;

    FbxVector4 lRotationVector, lNewPosition, lCurPosition;
    FbxAMatrix lRotation;
    FbxVector4 lCenter = pfbxCamera->InterestPosition.Get();

    // current position
    FbxVector4 lPosition = pfbxCamera->Position.Get();
    lCurPosition = lPosition-lCenter;

    // translate
    lNewPosition = lOrigCamPos-lCenter;

    int rotX;
    if (lNewPosition[2] == 0) {
        rotX = 90;
    } else {
        rotX = (int) (atan((double)lNewPosition[0]/(double)lNewPosition[2]) * FBXSDK_180_DIV_PI);
    }
    bool bRoll = (((int)OrigRoll % 360) != 0);
    if (   (lNewPosition[2] < 0 && !bRoll)
        || (lNewPosition[2] > 0 && bRoll) ) {
            dY = -dY;
    }
    if (bRoll) dX = -dX;

    // Center on the X axis (push)
    lRotationVector[1] = -rotX;
    lRotation.SetR(lRotationVector);
    lNewPosition = lRotation.MultT(lNewPosition);
    // Rotation for the vertical movement: around the X axis
    lRotationVector[1] = 0;
    lRotationVector[0] = dY;
    lRotation.SetR(lRotationVector);
    lNewPosition = lRotation.MultT(lNewPosition);
    // Back from the X axis (pop)
    lRotationVector[0] = 0;
    lRotationVector[1] = rotX;
    lRotation.SetR(lRotationVector);
    lNewPosition = lRotation.MultT(lNewPosition);
    // Rotation for the horizontal movement
    lRotationVector[1] = -dX;
    lRotation.SetR(lRotationVector);
    lNewPosition = lRotation.MultT(lNewPosition);

    // Detect camera flip
    if (   lNewPosition[0]*lCurPosition[0] < 0 
        && lNewPosition[2]*lCurPosition[2] < 0) {
            // flip -> roll 180.
            double lRoll = pfbxCamera->Roll.Get();
            lRoll = 180.0-lRoll;
            pfbxCamera->Roll.Set(lRoll);
    }

    // Back from center
    lNewPosition = lNewPosition + lCenter;

    pfbxCamera->Position.Set(lNewPosition);

}

void CameraPan(FbxScene *pfbxScene, FbxVector4 lOrigCamPos, FbxVector4 lOrigCamCenter, double OrigRoll, int dX, int dY) {
    // Pan the camera horizontally dX degrees, vertically dY degrees.
    FbxCamera* pfbxCamera = GetCurrentCamera(pfbxScene);
    if (!pfbxCamera) return;
    if (!IsProducerCamera(pfbxScene, pfbxCamera)) return;
    if (pfbxCamera->LockMode.Get()) return;
    if (dX == 0 && dY == 0) return;

    FbxGlobalCameraSettings& lGlobalCameraSettings = pfbxScene->GlobalCameraSettings();

    FbxVector4 lRotationXV, lRotationYV, lTranslationV;
    FbxAMatrix lRotationX, lRotationY, lRotationXInverse, lRotationYInverse, lTranslation;
    FbxVector4 lNewPosition = lOrigCamPos;
    FbxVector4 lNewCenter = lOrigCamCenter;

    // Translate the camera in dX and dY according to its point of view.
    if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerTop()) {
        lTranslationV[0] = -dX;
        lTranslationV[1] = 0;
        lTranslationV[2] = dY;
    } else if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerBottom()) {
        lTranslationV[0] = dX;
        lTranslationV[1] = 0;
        lTranslationV[2] = dY;
    } else if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerFront()) {
        lTranslationV[0] = -dX;
        lTranslationV[1] = -dY;
        lTranslationV[2] = 0;
    } else if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerBack()) {
        lTranslationV[0] = dX;
        lTranslationV[1] = -dY;
        lTranslationV[2] = 0;
    } else if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerRight()) {
        lTranslationV[0] = 0;
        lTranslationV[1] = -dY;
        lTranslationV[2] = dX;
    } else if (pfbxCamera == lGlobalCameraSettings.GetCameraProducerLeft()) {
        lTranslationV[0] = 0;
        lTranslationV[1] = -dY;
        lTranslationV[2] = -dX;
    } else {
        // Perspective view. More computation.

        // Adjust displacement when there is roll
        bool bRoll = (((int)OrigRoll % 360) != 0);
        if (bRoll) {
            dX = -dX;
            dY = -dY;
        }

        // Compute angles aY and aZ of the camera with Y and Z axis.
        double aZ, aY;
        /// Vector of distance between camera and center (lookat)
        FbxVector4 lDist = lNewPosition - lNewCenter;
        // Euclidian distance between camera and lookat
        double dist = (double)(lDist[0]*lDist[0]+lDist[1]*lDist[1]+lDist[2]*lDist[2]);
        // aZ
        if (lDist[2] == 0) {
            aZ = 90.0;
        } else {
            aZ = (atan((double)lDist[0]/(double)lDist[2]) * FBXSDK_180_DIV_PI);
        }
        if (lNewPosition[2] < lNewCenter[2]) aZ += 180;
        // aY
        if (dist > 0.001) {
            aY = (asin(sqrt((double)(lDist[1]*lDist[1])/ dist)) * FBXSDK_180_DIV_PI);
        } else {
            aY = 0;
        }
        if (lNewPosition[1] < lNewCenter[1]) aY = -aY;


        // Basis translation
        lTranslationV[0] = -dX;
        lTranslationV[1] = -dY;
        lTranslationV[2] = 0;

        // Rotation around Y axis
        lRotationYV[0] = 0;
        lRotationYV[1] = -aZ;
        lRotationYV[2] = 0;
        lRotationY.SetR(lRotationYV);
        // Rotation around X axis
        lRotationXV[0] = aY;
        lRotationXV[1] = 0;
        lRotationXV[2] = 0;
        lRotationX.SetR(lRotationXV);

        // Modify translation according to aY and aZ.
        lTranslation.SetT(lTranslationV);
        lRotationYInverse = lRotationY.Inverse();
        lRotationXInverse = lRotationX.Inverse();
        lTranslation = lRotationYInverse * lRotationXInverse * lTranslation * lRotationY * lRotationX;
        lTranslationV = lTranslation.GetT();
    }

    // Translate camera and center according to pan.
    lNewPosition += lTranslationV;
    lNewCenter   += lTranslationV;

    pfbxCamera->Position.Set(lNewPosition);
    pfbxCamera->InterestPosition.Set(lNewCenter);
}

