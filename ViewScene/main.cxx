#include "SceneContext.h"
#include "GL/glut.h"

void OnTimerCallback(int);
void OnDisplayCallback();
void OnKeyboardCallback(unsigned char pKey, int, int);
void OnMouseClickCallback(int button, int state, int x, int y);
void OnMouseMoveCallback(int x, int y);

CSceneContext *gpFbxSceneContext = NULL;

bool InitializeOpenGL()
{
	GLenum lError = ::glewInit();
	if (lError != GLEW_OK)
	{
		FBXSDK_printf("GLEW Error: %s\n", ::glewGetErrorString(lError));
		return(false);
	}

	::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	::glClearColor(0.0, 0.0, 0.0, 0.0);

	if (!GLEW_VERSION_1_5)
	{
		FBXSDK_printf("The OpenGL version should be at least 1.5 to display shaded scene!\n");
		return(false);
	}

	return(true);
}

int main(int argc, char** argv)
{
    ::glutInit(&argc, argv);
    ::glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    ::glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); 
    ::glutInitWindowPosition(100, 100);
    ::glutCreateWindow("ViewScene");

    InitializeOpenGL();

    ::glutDisplayFunc(OnDisplayCallback); 
    ::glutKeyboardFunc(OnKeyboardCallback);
    ::glutMouseFunc(OnMouseClickCallback);
    ::glutMotionFunc(OnMouseMoveCallback);

	gpFbxSceneContext = new CSceneContext();

	::glutTimerFunc(gpFbxSceneContext->GetFrameTimeByMilliSeconds(), OnTimerCallback, 0);

	::glutMainLoop();

	if (gpFbxSceneContext) delete gpFbxSceneContext;

    return(0);
}

void OnTimerCallback(int)
{
	glutPostRedisplay();
	gpFbxSceneContext->OnTimer();
	glutTimerFunc(gpFbxSceneContext->GetFrameTimeByMilliSeconds() * 2, OnTimerCallback, 0);
}

void OnDisplayCallback()
{
	gpFbxSceneContext->OnDisplay();
	glutSwapBuffers();
}

void OnKeyboardCallback(unsigned char nKey, int xPos, int yPos)
{
	if (nKey == 27) ::exit(0); //ESC
	else if (nKey == 'z') gpFbxSceneContext->m_nCameraStatus = CAMERA_ZOOM;
	else if (nKey == 'o') gpFbxSceneContext->m_nCameraStatus = CAMERA_ORBIT;
	else if (nKey == 'p') gpFbxSceneContext->m_nCameraStatus = CAMERA_PAN;
	else if ((nKey == '0') || (nKey == '1') || (nKey == '2')) gpFbxSceneContext->SetCurrentAnimStack(nKey - '0');
}

void OnMouseClickCallback(int nButton, int nState, int xPos, int yPos)
{
	gpFbxSceneContext->OnMouseClick(nButton, nState, xPos, yPos);
}

void OnMouseMoveCallback(int xPos, int yPos)
{
	gpFbxSceneContext->OnMouseMove(xPos, yPos);
}
