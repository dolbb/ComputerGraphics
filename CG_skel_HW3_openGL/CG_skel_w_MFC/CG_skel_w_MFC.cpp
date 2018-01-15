// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "TrackBall.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>
#include "InputDialog.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2

#define DEFAULT_WIDTH 512
#define DEFAULT_HEIGHT 512

Scene *scene;

/*	our current state parameters:		*/
int last_x,last_y;
int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;
bool lb_down = false;
bool rb_down = false;
bool mb_down = false;
int modifier;
bool redraw = false;
OperateParams operationParams;


TrackBall trackBall(DEFAULT_WIDTH, DEFAULT_HEIGHT);
//----------------------------------------------------------------------------
// Callbacks

static void updateOrientation()
{
	vec3 translationVec;
	vec3 rotationAxis;
	vec3 zeroVec(0.0);
	vec3 scalingVector;
	float angle;
	switch (modifier)
	{
	case GLUT_ACTIVE_SHIFT:
		//translation
		translationVec = trackBall.getTranslation();
		if (!(translationVec == zeroVec))
		{
			//TODO: OPERATE WITH SCENE
		}
		break;

	case GLUT_ACTIVE_CTRL:
		//non uniform scaling
		scalingVector = trackBall.getNonUniformScaling();
		break;

	case GLUT_ACTIVE_ALT:
		//uniform scaling
		scalingVector = trackBall.getUniformScaling();
		break;

	default:
		//rotation
		rotationAxis = trackBall.getRotationAxis();
		angle = trackBall.getRotationAngle();
		if (!isnan(angle))
		{
			//TODO: OPERATE WITH SCENE
		}
		break;
	}
}

void display( void )
{
	if (lb_down)
	{
		updateOrientation();
		trackBall.setStart(last_x, last_y);
	}
	scene->draw();
}

void idle()
{
	glutPostRedisplay();
}

void reshape( int newWidth, int newHeight )
{
	GLfloat heightRatioChage = (GLfloat)newHeight / (GLfloat)height;
	GLfloat widthRatioChange = (GLfloat)newWidth / (GLfloat)width;
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
	//TODO:update projection spec for display:
	//scene->changeProjectionRatio(widthRatioChange, heightRatioChage);
	trackBall.setViewport(width, height);
}

void keyboard( unsigned char key, int x, int y )
{
	switch ( key ) {
	case 033:
		exit( EXIT_SUCCESS );
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	modifier = glutGetModifiers();
	
	//set down flags
	switch(button) {
		case GLUT_LEFT_BUTTON:
			lb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_RIGHT_BUTTON:
			rb_down = (state==GLUT_UP)?0:1;
			break;
		case GLUT_MIDDLE_BUTTON:
			mb_down = (state==GLUT_UP)?0:1;	
			break;
	}

	if (lb_down)
	{
		trackBall.setStart(x, y);
	}
	if (!lb_down)
	{
		trackBall.resetPoints();
	}
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	//update ending point for the trackball to calculate transformation
	if (dx || dy)
	{
		trackBall.setEnd(x, y);
	}
	// update last x,y
	last_x=x;
	last_y=y;
}
/*void special(int key, int x, int y)
{
	int modifier = glutGetModifiers();
	operationParams.type = ROTATE;
	if (modifier == GLUT_ACTIVE_SHIFT)
	{
		parameters.type = TRANSLATE;
	}
	parameters.frame = currentCameraFrame;
	switch (key)
	{
	case GLUT_KEY_UP:
		//set translation parameters
		if (parameters.type == TRANSLATE)
		{
			if (direction == elevated)
			{
				parameters.v = vec3(0, 1, 0)*DEFAULT_TRANSLATION*transformationFactor;
			}
			else
			{
				parameters.v = vec3(0, 0, -1)*DEFAULT_TRANSLATION*transformationFactor;
			}
		}
		//set rotation parameters
		else
		{
			parameters.v = vec3(1, 0, 0)*DEFAULT_ANGLE*transformationFactor;
		}
		break;

	case GLUT_KEY_DOWN:
		//set translation parameters
		if (parameters.type == TRANSLATE)
		{
			if (direction == elevated)
			{
				parameters.v = vec3(0, -1, 0)*DEFAULT_TRANSLATION*transformationFactor;
			}
			else
			{
				parameters.v = vec3(0, 0, 1)*DEFAULT_TRANSLATION*transformationFactor;
			}
		}
		//set rotation parameters
		else
		{
			parameters.v = vec3(-1, 0, 0)*DEFAULT_ANGLE*transformationFactor;
		}
		break;

	case GLUT_KEY_LEFT:
		//set translation parameters
		if (parameters.type == TRANSLATE)
		{
			parameters.v = vec3(-1, 0, 0)*DEFAULT_TRANSLATION*transformationFactor;
		}
		//set rotation parameters
		else
		{
			parameters.v = vec3(0, 1, 0)*DEFAULT_ANGLE*transformationFactor;
		}
		break;

	case GLUT_KEY_RIGHT:
		//set translation parameters
		if (parameters.type == TRANSLATE)
		{
			parameters.v = vec3(1, 0, 0)*DEFAULT_TRANSLATION*transformationFactor;
		}
		//set rotation parameters
		else
		{
			parameters.v = vec3(0, -1, 0)*DEFAULT_ANGLE*transformationFactor;
		}
		break;
	}
	scene->operate(parameters);
	redraw = true;
}*/

void fileMenu(int id)
{
	switch (id)
	{
		case FILE_OPEN:
			CFileDialog dlg(TRUE,_T(".obj"),NULL,NULL,_T("*.obj|*.*"));
			if(dlg.DoModal()==IDOK)
			{
				std::string s((LPCTSTR)dlg.GetPathName());
				scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
			}
			else
			{
				return;
			}
			break;
	}
}

void mainMenu(int id)
{
	switch (id)
	{
	case MAIN_DEMO:
		scene->drawDemo();
		break;
	case MAIN_ABOUT:
		AfxMessageBox(_T("Computer Graphics"));
		break;
	}
}

void initMenu()
{

	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddMenuEntry("Demo",MAIN_DEMO);
	glutAddMenuEntry("About",MAIN_ABOUT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------



int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow( "Computer Graphics - HW3" );
	glewExperimental = GL_TRUE;
	glewInit();
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		/*		...*/
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	scene = new Scene;
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutIdleFunc(idle); 
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	
	/*added: *///TODO: ADD THE ADDED
	//glutSpecialFunc(special);
	//glutMouseWheelFunc(mouseWheel);
	
	initMenu();

	glutMainLoop();
	delete scene;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
{
	int nRetCode = 0;
	
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		my_main(argc, argv );
	}
	
	return nRetCode;
}
