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
#include "UnitTest.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

enum mainMenuIdentifier{DEMO};
enum newMenuIdentifier{NEW_MODEL, NEW_CAMERA};
enum selectMenuIdentifier{ACTIVE_MODEL};
enum toolsMenuIdentifier{LOOKAT_ACTIVE_MODEL, SET_TRANSFORMATION_STEP,SET_CAMERA_PRESPECTIVE};
enum toggleMenuIdentifier{FACE_NORMALS, VERTEX_NORMALS, BOUNDING_BOX, CAMERA_RENDERING};
enum defaultStepSize{DEFAULT_DX=1, DEFAULT_DY=1};
enum numOfFrames{OBJECT_FRAMES=2, CAMERA_FRAMES=2};

#define DEFAULT_ZOOM 1.5
#define DEFAULT_SCALING_FACTOR 1.5

Scene *scene;
Renderer *renderer;

int last_x,last_y;
int width = DEFAULT_SCREEN_X;
int height = DEFAULT_SCREEN_Y;
bool lb_down,rb_down,mb_down;
Frames currentObjectFrame;
Frames currentCameraFrame;
OperationType currentOperation;
int transformationFactor = 1;
vec3 startingPoint;
vec3 endingPoint;
vec3 rotationAxis;
GLfloat angle;
const float EPSILON = FLT_MIN;
//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
	scene->draw();
}

void reshape( int newWidth, int newHeight )
{
	width = newWidth;
	height = newHeight;
	renderer->resizeBuffers(width, height);
	scene->draw();
}

const vec3& getParameters()
{
	CXyzDialog dialogue;
	if (dialogue.DoModal() == IDOK)
	{
		return dialogue.GetXYZ();
	}
}

void keyboard( unsigned char key, int x, int y )
{
	vec3 transformationParameters;
	switch (key)
	{
		//ESC
		case 033:
			exit(EXIT_SUCCESS);
		break;

		//swap object transformation frame
		case 'm':
			cout << "current object transformation frame: ";
			if (currentObjectFrame == MODEL)
			{
				currentObjectFrame = WORLD;
				cout << "world" << endl;
			}
			else
			{
				currentObjectFrame = MODEL;
				cout << "model" << endl;
			}
		break;

		//swap model transformation frame
		case 'c':
			cout << "current camera transformation frame: ";
			if (currentCameraFrame == CAMERA_POSITION)
			{
				currentCameraFrame = CAMERA_VIEW;
				cout << "view" << endl;
			}
			else
			{
				currentCameraFrame = CAMERA_POSITION;
				cout << "world" << endl;
			}
		break;

		case 'r':
			transformationParameters = getParameters();
			scene->operate(ROTATE, 1, 1, currentObjectFrame, transformationParameters*transformationFactor);
		break;

		case 's':
			transformationParameters = getParameters();
			scene->operate(SCALE, 1, 1, currentObjectFrame, transformationParameters*transformationFactor);
		break;

		case 't':
			transformationParameters = getParameters();
			scene->operate(TRANSLATE, 1, 1, currentObjectFrame, transformationParameters*transformationFactor);
	}
}

void mouseWheel(int wheel, int direction, int x, int y)
{
	vec3 defaultVector;
	if (direction > 0)
	{
		scene->operate(UNIFORM_SCALE, DEFAULT_ZOOM, 1, ZOOM,defaultVector);
	}
	else
	{
		scene->operate(UNIFORM_SCALE, 1, DEFAULT_ZOOM, ZOOM, defaultVector);
	}
}

vec3 projectToSphere(int x, int y)
{
	/*
		points in xy plane are projected back to the canonical view volume, a sphere with radius 1 is imagined behind
		the xy plane.
	*/
	GLfloat projectedX = (2.0 * x - width) / width;
	GLfloat projectedY = (2.0*y - height) / height;
	vec2    projectedXY(projectedX, projectedY);
	GLfloat d = length(projectedXY);
	GLfloat projectedZ;
	if (d <= 1)
	{
		projectedZ = sqrt(1 - d);
	}
	else
	{
		normalize(projectedXY);
		d = length(projectedXY);
		projectedZ = sqrt(1 - d);
	}
	return normalize(vec3(projectedXY, projectedZ));
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
	if (lb_down)
	{
		if (x >= width || y >= height || x < 0 || y < 0) return;
		startingPoint = projectToSphere(x, y);
	}
	if (!lb_down)
	{
		if (x >= width || y >= height || x < 0 || y < 0) return;
		endingPoint = projectToSphere(x, y);
		rotationAxis = cross(startingPoint, endingPoint);
		angle = asin(length(rotationAxis) / length(startingPoint)*length(endingPoint));
		angle = (angle * 180) / M_PI;
		scene->operate(ROTATE, angle, angle, currentObjectFrame, rotationAxis);
	}
}

void special(int key, int x, int y)
{
	vec3 defaultVector;
	switch (key)
	{
	case GLUT_KEY_UP:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, 0, DEFAULT_DY*transformationFactor, currentCameraFrame,defaultVector);
		}
		else
		{
			scene->operate(TRANSLATE, 0, DEFAULT_DY*transformationFactor, currentCameraFrame, defaultVector);
		}
	break;

	case GLUT_KEY_DOWN:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, 0, -(DEFAULT_DY*transformationFactor), currentCameraFrame, defaultVector);
		}
		else
		{
			scene->operate(TRANSLATE, 0, -(DEFAULT_DY*transformationFactor), currentCameraFrame, defaultVector);
		}
	break;

	case GLUT_KEY_RIGHT:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, DEFAULT_DX*transformationFactor, 0, currentCameraFrame, defaultVector);
		}
		else
		{
			scene->operate(TRANSLATE, DEFAULT_DX*transformationFactor, 0, currentCameraFrame, defaultVector);
		}
	break;

	case GLUT_KEY_LEFT:
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			scene->operate(ROTATE, -(DEFAULT_DX*transformationFactor), 0, currentCameraFrame, defaultVector);
		}
		else
		{
			scene->operate(TRANSLATE, -(DEFAULT_DX*transformationFactor), 0, currentCameraFrame, defaultVector);
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
	vec3 defaultVector;
	vec3 transformationParameters;
	switch (modifier)
	{
		case GLUT_ACTIVE_CTRL:
			if (dy >= 0)
			{
				scene->operate(UNIFORM_SCALE, DEFAULT_SCALING_FACTOR*transformationFactor, 1, currentObjectFrame, defaultVector);
			}
			else
			{
				scene->operate(UNIFORM_SCALE, 1, DEFAULT_SCALING_FACTOR*transformationFactor, currentObjectFrame,defaultVector);
			}
		break;
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
		string message = "Availible models: ";
		for (int i = 0; i < modelNames.size(); i++)
		{
			message += modelNames[i];
			message += " , ";
		}
		string chosenName;
		CCmdDialog name(message.c_str());
		while (find(modelNames.begin(), modelNames.end(), chosenName) == modelNames.end())
		{
			if (name.DoModal() == IDOK)
			{
				chosenName = name.GetCmd();
				//keep asking the user for input as long as the chosen name does not exist
			}
		}
		scene->selectActiveModel(chosenName);
	}
}

void setTransformationStep()
{
	string num;
	int scannedNum=1;
	bool scanned=false;
	CCmdDialog step("Please enter a positive number");
	while (!scanned)
	{
		if (step.DoModal() == IDOK)
		{
			try
			{
				num = step.GetCmd();
				scannedNum = stoi(num);
				//stoi can throw invalid_argument and out_of_range exceptions
			}
			//in case of an exception the scan was unsucessful
			catch (...)
			{
				continue; 
			}
			if (scannedNum <= 0)
			{
				continue;
			}
			scanned = true;
		}
	}
	transformationFactor = scannedNum;
}

void setCameraPerspective()
{

}

void mainMenu(int id)
{
	if (id == DEMO)
	{
		scene->drawDemo();
	}
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

	case CAMERA_RENDERING:
		//scene->drawActiveCamera();
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
	glutAddMenuEntry("Vertex normals", VERTEX_NORMALS);
	glutAddMenuEntry("Bounding box", BOUNDING_BOX);
	glutAddMenuEntry("Camera rendering", CAMERA_RENDERING);
	int toolsMenu = glutCreateMenu(toolsMenuCallback);
	glutAddMenuEntry("LookAt active model", LOOKAT_ACTIVE_MODEL);
	glutAddMenuEntry("Set transformation step", SET_TRANSFORMATION_STEP);
	glutAddMenuEntry("Set camera perspective", SET_CAMERA_PRESPECTIVE);
	glutAddSubMenu("Toggle", toggleMenu);
	glutCreateMenu(mainMenu);
	glutAddSubMenu("New", newMenu);
	glutAddSubMenu("Select", selectMenu);
	glutAddSubMenu("Tools", toolsMenu);
	glutAddMenuEntry("Demo", DEMO);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//----------------------------------------------------------------------------



int my_main( int argc, char **argv )
{
	//----------------------------------------------------------------------------
	// Initialize window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA| GLUT_DOUBLE);
	glutInitWindowSize(DEFAULT_SCREEN_X, DEFAULT_SCREEN_Y);
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
	currentCameraFrame = CAMERA_VIEW;
	currentObjectFrame = WORLD;
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

	/*=======================================
					TEST BELOW					
	=======================================*/
	//mainOverallTest();
	//return 0;
}
