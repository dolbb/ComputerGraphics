// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"


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

//main menu constants
#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2
#define SET_FACTOR 3
#define RESET_FACTOR 4
#define CREATE_CAMERA 5
#define SET_PROJECTION 6
//toggle menu constants
#define BOUNDING_BOX 0
#define VERTEX_NORMALS 1
#define FACE_NORMALS 2
//transformation operations factors
#define SCALING_PARAMETER 2
#define MOVEMENT_PARAMETER 1
#define ROTATION_PARAMETER 1

Scene *scene;
Renderer *renderer;

int last_x,last_y;
bool lb_down,rb_down,mb_down;
Frames currentFrame;
OperationType currentOperation;
int transformationFactor = 1;
//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
	scene->draw();
}

void reshape( int width, int height )
{
	renderer->resizeBuffers(width, height);
	scene->refreshView();
	scene->draw();
}

void keyboard( unsigned char key, int x, int y )
{
	switch ( key ) {
	//ESC
	case 033:
		exit( EXIT_SUCCESS );
		break;
		//active model selection
	case 'z':
		scene->selectActiveModel();
		break;
		//create new camera
	case 'c':
		scene->selectActiveCamera();
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//button = {GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON}
	//state = {GLUT_DOWN,GLUT_UP}
	
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
	// add your code
}

void special(int key, int x, int y)
{
	if (currentFrame == CAMERA_POSITION || currentFrame == CAMERA_VIEW)
	{
		if (key == GLUT_KEY_LEFT)
		{	
			if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			{
				scene->operate(ROTATE, 0, -ROTATION_PARAMETER, currentFrame);
			}
			else
			{
				scene->operate(TRANSLATE, -MOVEMENT_PARAMETER, 0, currentFrame);
			}
		}
		if (key == GLUT_KEY_RIGHT)
		{
			if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			{
				scene->operate(ROTATE, 0, ROTATION_PARAMETER, currentFrame);
			}
			else
			{
				scene->operate(TRANSLATE, MOVEMENT_PARAMETER, 0, currentFrame);
			}
		}
		if (key == GLUT_KEY_UP)
		{
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
			{
				scene->operate(SCALE, SCALING_PARAMETER, SCALING_PARAMETER, currentFrame);
			}
			else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			{
				scene->operate(ROTATE, ROTATION_PARAMETER, 0, currentFrame);
			}
			else
			{
				scene->operate(TRANSLATE, 0, MOVEMENT_PARAMETER, currentFrame);
			}
		}
		if (key == GLUT_KEY_DOWN)
		{
			if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
			{
				scene->operate(SCALE, 1/SCALING_PARAMETER, 1/SCALING_PARAMETER, currentFrame);
			}
			else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
			{
				scene->operate(ROTATE, -ROTATION_PARAMETER, 0, currentFrame);
			}
			else
			{
				scene->operate(TRANSLATE, 0, -MOVEMENT_PARAMETER, currentFrame);
			}
		}
	}
}

void motion(int x, int y)
{
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
	if (currentFrame == MODEL || currentFrame == WORLD)
	{
		//held shift mouse drag - non uniform scaling
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(SCALE, dx, dy, currentFrame);
		}
		//held ctlr mouse drag - uniform scaling only the x axis plays part in uniform scaling
		else if (glutGetModifiers() == GLUT_ACTIVE_CTRL)
		{
			scene->operate(SCALE, dx, dx, currentFrame);
		}
		//held alt mouse drag - rotate
		else if (glutGetModifiers() == GLUT_ACTIVE_ALT)
		{
			scene->operate(ROTATE, dx, dy, currentFrame);
		}
		//regular mouse drag - translate
		else
		{
			scene->operate(TRANSLATE, dx, dy, currentFrame);
		}
	}
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

//TODO: AFTER THE SYNC, REMOVE COMMENTS
void toggleSelection(int id)
{
	switch(id)
	{
	case BOUNDING_BOX:
		scene->featuresStateSelection(TOGGLE_BOUNDING_BOX);
		break;

	case VERTEX_NORMALS:
		scene->featuresStateSelection(TOGGLE_VERTEX_NORMALS);
		break;

	case FACE_NORMALS:
		scene->featuresStateSelection(TOGGLE_FACE_NORMALS);
		break;
	}
}


void objectFrame(int id)
{
	switch (id)
	{
	case MODEL:
		currentFrame = MODEL;
		break;
	case WORLD:
		currentFrame = WORLD;
		break;
	}
}

void cameraFrame(int id)
{
	switch (id)
	{
	case CAMERA_POSITION:
		currentFrame = CAMERA_POSITION;
		break;
	case CAMERA_VIEW:
		currentFrame = CAMERA_VIEW;
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
	
	case SET_FACTOR:
		cout << "Please enter a positive integer value representing your desired transformation factor: ";
		cin >> transformationFactor;
		while (transformationFactor <= 0)
		{
			cout << "Please enter a positive integer: ";
			cin >> transformationFactor;
		}
		cout << "Your new transformation factor is: " << transformationFactor << endl;
	break;

	case RESET_FACTOR:
		transformationFactor = 1;
		cout << "Your new transformation factor is: " << transformationFactor << endl;
	break;

	case CREATE_CAMERA:
		scene->createCamera();
		break;
	}
}


void initMenu()
{
	int menuFile = glutCreateMenu(fileMenu);
	glutAddMenuEntry("Open..",FILE_OPEN);
	int objectTransformMenu = glutCreateMenu(objectFrame);
	glutAddMenuEntry("model frame", MODEL);
	glutAddMenuEntry("world frame", WORLD);
	int cameraTransformMenu = glutCreateMenu(cameraFrame);
	glutAddMenuEntry("world frame", CAMERA_POSITION);
	glutAddMenuEntry("view frame", CAMERA_VIEW);
	int menuToggle = glutCreateMenu(toggleSelection);
	glutAddMenuEntry("Face normals", FACE_NORMALS);
	glutAddMenuEntry("Bounding box", BOUNDING_BOX);
	glutAddMenuEntry("Vertex normals", VERTEX_NORMALS);
	glutCreateMenu(mainMenu);
	glutAddSubMenu("File",menuFile);
	glutAddSubMenu("Toggle", menuToggle);
	glutAddSubMenu("Object transform frame", objectTransformMenu);
	glutAddSubMenu("Camera transform frame", cameraTransformMenu);
	glutAddMenuEntry("Set transformation factor", SET_FACTOR);
	glutAddMenuEntry("Reset transformation factor", RESET_FACTOR);
	glutAddMenuEntry("Create new camera", CREATE_CAMERA);
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
	glutInitWindowSize( 512, 512 );
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

	renderer = new Renderer();
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
