// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "TrackBall.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "vec.h"
#include "mat.h"
#include "InitShader.h"
#include "Scene.h"
#include "Renderer.h"
#include <string>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2

#define DEFAULT_WIDTH 512
#define DEFAULT_HEIGHT 512

Scene *scene;
Renderer *renderer;

int last_x,last_y;
int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;
bool lb_down = false;
bool rb_down = false;
bool mb_down = false;
int modifier;

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
	//TODO: OPERATE WITH SCENE AND RENDERER
	GLfloat heightRatioChage = (GLfloat)newHeight / (GLfloat)height;
	GLfloat widthRatioChange = (GLfloat)newWidth / (GLfloat)width;
	//scene->changeProjectionRatio(widthRatioChange, heightRatioChage);
	width = newWidth;
	height = newHeight;
	//renderer->resizeBuffers(width, height);
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
	glutCreateWindow( "CG" );
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

	
	
	renderer = new Renderer(DEFAULT_WIDTH,DEFAULT_HEIGHT);
	scene = new Scene(renderer);
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	initMenu();

	glutMainLoop();
	delete scene;
	delete renderer;
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
