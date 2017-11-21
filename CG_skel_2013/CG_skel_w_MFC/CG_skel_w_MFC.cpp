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
#include <algorithm>

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

enum newMenuIdentifier{NEW_MODEL, NEW_CAMERA};
enum selectMenuIdentifier{ACTIVE_MODEL};
enum toolsMenuIdentifier{LOOKAT_ACTIVE_MODEL, SET_TRANSFORMATION_STEP,SET_CAMERA_PRESPECTIVE};
enum toggleMenuIdentifier{FACE_NORMALS, VERTEX_NORMALS, BOUNDING_BOX, CAMERA_RENDERING};
enum defaultStepSize{DEFAULT_DX=1, DEFAULT_DY=1};
enum numOfFrames{OBJECT_FRAMES=2, CAMERA_FRAMES=2};

#define DEFAULT_ZOOM 1.5

Scene *scene;
Renderer *renderer;

int last_x,last_y;
bool lb_down,rb_down,mb_down;
Frames currentObjectFrame;
Frames currentCameraFrame;
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
	switch (key)
	{
		//ESC
		case 033:
			exit(EXIT_SUCCESS);
		break;

		//swap object transformation frame
		case 'm':
			if (currentObjectFrame == MODEL)
			{
				currentObjectFrame = WORLD;
			}
			else
			{
				currentObjectFrame = MODEL;
			}
		break;

		//swap model transformation frame
		case 'c':
			if (currentCameraFrame == CAMERA_POSITION)
			{
				currentCameraFrame = CAMERA_VIEW;
			}
			else
			{
				currentCameraFrame = CAMERA_POSITION;
			}
		break;
	}
}

void mouseWheel(int wheel, int direction, int x, int y)
{
	if (direction > 0)
	{
		scene->operate(UNIFORM_SCALE, DEFAULT_ZOOM, DEFAULT_ZOOM, ZOOM);
	}
	else
	{
		scene->operate(UNIFORM_SCALE, 1 / DEFAULT_ZOOM, 1 / DEFAULT_ZOOM, ZOOM);
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
}

void special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, 0, DEFAULT_DY*transformationFactor, currentCameraFrame);
		}
		else
		{
			scene->operate(TRANSLATE, 0, DEFAULT_DY*transformationFactor, currentCameraFrame);
		}
	break;

	case GLUT_KEY_DOWN:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, 0, -(DEFAULT_DY*transformationFactor), currentCameraFrame);
		}
		else
		{
			scene->operate(TRANSLATE, 0, -(DEFAULT_DY*transformationFactor), currentCameraFrame);
		}
	break;

	case GLUT_KEY_RIGHT:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, DEFAULT_DX*transformationFactor, 0, currentCameraFrame);
		}
		else
		{
			scene->operate(TRANSLATE, DEFAULT_DX*transformationFactor, 0, currentCameraFrame);
		}
	break;

	case GLUT_KEY_LEFT:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, -(DEFAULT_DX*transformationFactor), 0, currentCameraFrame);
		}
		else
		{
			scene->operate(TRANSLATE, -(DEFAULT_DX*transformationFactor), 0, currentCameraFrame);
		}
	break;

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
	int modifier = glutGetModifiers();
	switch (modifier)
	{
		case GLUT_ACTIVE_SHIFT:
			scene->operate(ROTATE, dx*transformationFactor, dy*transformationFactor, currentObjectFrame);
		break;

		case GLUT_ACTIVE_CTRL:
			scene->operate(UNIFORM_SCALE, dy*transformationFactor, dy*transformationFactor, currentObjectFrame);
		break;
		
		case GLUT_ACTIVE_ALT:
			scene->operate(SCALE, dx*transformationFactor, dy*transformationFactor, currentObjectFrame);
		break;
		
		default:
			scene->operate(TRANSLATE, dx*transformationFactor, dy*transformationFactor, currentObjectFrame);
	}
}

void newMenuCallback(int id)
{
	if (id == NEW_MODEL)
	{
		CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
		if (dlg.DoModal() == IDOK)
		{
			std::string s((LPCTSTR)dlg.GetPathName());
			scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
		}
	}
	if(id==NEW_CAMERA)
	{
		scene->createCamera();
	}
}

void selectMenuCallback(int id)
{
	if (id == ACTIVE_MODEL)
	{
		vector<string>& modelNames = scene->getModelNames();
		string message = "The scene has the following models: \n";
		for (int i = 0; i < modelNames.size(); i++)
		{
			message += modelNames[i];
			message += "\n";
		}
		string chosenName;
		CCmdDialog name(message.c_str());
		if (name.DoModal() == IDOK)
		{
			do
			{
				chosenName = name.GetCmd();
				//keep asking the user for input as long as the chosen name does not exist
			}	
			while (find(modelNames.begin(), modelNames.end(), chosenName) == modelNames.end());
		}
		scene->selectActiveModel(chosenName);
	}
}

void setTransformationStep()
{
	CCmdDialog step("Please enter a positive number");
	string num;
	int scannedNum;
	bool scanned;
	if (step.DoModal() == IDOK)
	{
		do
		{
			scanned = true;
			try
			{
				num = step.GetCmd();
				scannedNum = stoi(num);
				//stoi can throw invalid_argument and out_of_range exceptions
			}
			//in case of an exception the scan was unsucessful
			catch (invalid_argument)
			{
				scanned = false;
			}
			catch (out_of_range)
			{
				scanned = false;
			}
		} while (!scanned);
	}
	transformationFactor = scannedNum;
}

void setCameraPerspective()
{

}

void toolsMenuCallback(int id)
{
	if (id == LOOKAT_ACTIVE_MODEL)
	{
		scene->LookAtActiveModel();
	}
	
	if (id == SET_TRANSFORMATION_STEP)
	{
		setTransformationStep();
	}

	if (id == SET_CAMERA_PRESPECTIVE)
	{
		setCameraPerspective();
	}
}

void toggleMenuCallback(int id)
{
	switch (id)
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


void initMenu()
{
	int newMenu = glutCreateMenu(newMenuCallback);
	glutAddMenuEntry("Model", NEW_MODEL);
	glutAddMenuEntry("Camera", NEW_CAMERA);
	int selectMenu = glutCreateMenu(selectMenuCallback);
	glutAddMenuEntry("Active model", ACTIVE_MODEL);
	int toggleMenu = glutCreateMenu(toggleMenuCallback);
	glutAddMenuEntry("Face normals", FACE_NORMALS);
	glutAddMenuEntry("Vertex normals", FACE_NORMALS);
	glutAddMenuEntry("Bounding box", FACE_NORMALS);
	glutAddMenuEntry("Camera rendering", FACE_NORMALS);
	int toolsMenu = glutCreateMenu(toolsMenuCallback);
	glutAddMenuEntry("LookAt active model", LOOKAT_ACTIVE_MODEL);
	glutAddMenuEntry("Set transformation step", SET_TRANSFORMATION_STEP);
	glutAddMenuEntry("Set camera perspective", SET_CAMERA_PRESPECTIVE);
	glutAddSubMenu("Toggle", toggleMenu);
	glutCreateMenu(NULL);
	glutAddSubMenu("New", newMenu);
	glutAddSubMenu("Select", selectMenu);
	glutAddSubMenu("Tools", toolsMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------



int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize(DEFAULT_SCREEN_SIZE, DEFAULT_SCREEN_SIZE);
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
	glutMouseWheelFunc( mouseWheel );
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
