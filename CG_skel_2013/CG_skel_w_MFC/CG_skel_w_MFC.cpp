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

enum DebugMode{ON, OFF};
enum mainMenuIdentifier{DEMO};
enum newMenuIdentifier{ NEW_MODEL, NEW_CAMERA, NEW_PYRAMID, NEW_DEFAULT_LIGHT, NEW_CUSTOM_LIGHT };
enum selectMenuIdentifier{ACTIVE_MODEL};
enum toolsMenuIdentifier{ LOOKAT_ACTIVE_MODEL, SET_TRANSFORMATION_STEP, SET_CAMERA_PRESPECTIVE, SET_MODEL_COLOR, SET_LIGHT_COLOR, SET_LIGHT_DIRECTION, SET_LIGHT_INTENSITY };
enum toggleMenuIdentifier{ FACE_NORMALS, VERTEX_NORMALS, BOUNDING_BOX, CAMERA_RENDERING, KEYBOARD_MODE, ACTIVE_LIGHT_TYPE };
enum defaultStepSize{DEFAULT_DX=1, DEFAULT_DY=1};
enum numOfFrames{OBJECT_FRAMES=2, CAMERA_FRAMES=2};
enum scalingAxis{xT,yT,zT,uniformT};
enum cameraDirection{elevated, nonElevated};
enum KeyboardMode{CG1,CG2};

#define DEFAULT_ZOOM 1.2
#define DEFAULT_SCALING_FACTOR 1.2

Scene *scene;
Renderer *renderer;
KeyboardMode keyboardMode;

//	mouse call back parameters
int last_x,last_y;
bool lb_down, rb_down, mb_down;

//	window width and height
int width = DEFAULT_SCREEN_X;
int height = DEFAULT_SCREEN_Y;

//	track ball functions parameters

#define DEFAULT_ANGLE 10.0
#define DEFAULT_TRANSLATION 0.5
vec3 startingPoint;
vec3 endingPoint;
vec3 rotationAxis;

//	transformation call back functions parameters

int transformationFactor = 1;
const float EPSILON = FLT_MIN;
vec3 defaultVector;
vec3 translationVector;
GLfloat angle;
Frames currentObjectFrame;
Frames currentCameraFrame;
OperationType currentOperation;
OperateParams parameters;
scalingAxis currentAxis=uniformT;
cameraDirection direction = nonElevated;
ProjectionParams projection;

//	idle flags signalling if a move has been made by the user and a redraw is needed
bool redraw = false;

//----------------------------------------------------------------------------
// Callbacks

void display( void )
{
	scene->draw();
}

void idle()
{
	if (redraw)
	{
		glutPostRedisplay();
		redraw = false;
	}
}

void reshape( int newWidth, int newHeight )
{
	GLfloat heightRatioChage = (GLfloat)newHeight / (GLfloat)height;
	GLfloat widthRatioChange = (GLfloat)newWidth / (GLfloat)width;
	scene->changeProjectionRatio(widthRatioChange, heightRatioChage);
	width = newWidth;
	height = newHeight;
	renderer->resizeBuffers(width, height);
	redraw = true;
}

void CG2keyboard(unsigned char key, int x, int y){
	redraw = true;
	if (key >= '0' && key <= '9')
	{
		int index = key - '0';
		scene->switchActiveLight(index);
	}
	switch (key)
	{
	//ESC
	case 033:
		exit(EXIT_SUCCESS);
		break;
	//change light spec (inc \ dec) in a specific aspect:
	case 'e':
		scene->activeLightIncrementStats(AMBIENT);
		cout << "current light was incremented in its AMBIENT field" << endl;
		break;
	case 'd':
		scene->activeLightDecrementStats(AMBIENT);
		cout << "current light was decremented in its AMBIENT field" << endl;
		break;
	case 'r':
		scene->activeLightIncrementStats(DIFFUSE);
		cout << "current light was incremented in its DIFFUSE field" << endl;
		break;
	case 'f':
		scene->activeLightDecrementStats(DIFFUSE);
		cout << "current light was decremented in its DIFFUSE field" << endl;
		break;
	case 't':
		scene->activeLightIncrementStats(SPECULAR);
		cout << "current light was incremented in its SPECULAR field" << endl;
		break;
	case 'g':
		scene->activeLightDecrementStats(SPECULAR);
		cout << "current light was decremented in its SPECULAR field" << endl;
		break;
	}
}

void CG1keyboard(unsigned char key, int x, int y)
{
	redraw = true;
	vec3 transformationParameters;
	if (key >= '0' && key <= '9')
	{
		int index = key - '0';
		scene->selectActiveCamera(index);
	}
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

		case 'x':
			currentAxis = xT;
			cout << "current transformation axis: X" << endl;
		break;

		case 'y':
			currentAxis = yT;
			cout << "current transformation axis: Y" << endl;
		break;

		case 'z':
			currentAxis = zT;
			cout << "current transformation axis: Z" << endl;
		break;

		case 'u':
			currentAxis = uniformT;
			cout << "current transformation axis: uniform" << endl;
		break;

		case 'e':
			if (direction == nonElevated)
			{
				direction = elevated;
				cout << "the camera is now elevating in scene" << endl;
			}
			else
			{
				direction = nonElevated;
				cout << "the camera is now not elevating in scene" << endl;
			}
		break;
	}
}

void keyboard(unsigned char key, int x, int y){
	switch (keyboardMode){
	case CG1:
		CG1keyboard(key, x, y);
		break;
	case CG2:
		CG2keyboard(key, x, y);
		break;
	}
}

void mouseWheel(int wheel, int direction, int x, int y)
{
	redraw = true;
	parameters.frame = currentCameraFrame;
	parameters.type = SCALE;
	if (direction > 0)
	{
		parameters.uScale = DEFAULT_ZOOM*transformationFactor;
		scene->operate(parameters);
	}
	else
	{
		parameters.uScale = 1 / (DEFAULT_ZOOM*transformationFactor);
		scene->operate(parameters);
	}
}

vec3 projectToSphere(int x, int y)
{
	/*
		points in xy plane are projected back to the canonical view volume, a sphere with radius 1 is imagined behind
		the xy plane.
	*/
	GLfloat projectedX = (2.0 * x - width) / width;
	GLfloat projectedY = (height - 2.0*y) / height;
	vec2    projectedXY(projectedX, projectedY);
	GLfloat d = length(projectedXY);
	GLfloat projectedZ;
	if (d < 1)
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
		startingPoint = projectToSphere(x, y);
	}
}

void special(int key, int x, int y)
{
	redraw = true;
	int modifier = glutGetModifiers();
	parameters.type = TRANSLATE;
	if (modifier == GLUT_ACTIVE_SHIFT)
	{
		parameters.type = ROTATE;
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
			parameters.v = vec3(0, 0, 1)*DEFAULT_ANGLE*transformationFactor;
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
			parameters.v = vec3(0, 0, -1)*DEFAULT_ANGLE*transformationFactor;
		}
	break;
	}
	scene->operate(parameters);
}

void rotate(int x, int y, int dy)
{
	parameters.type = ROTATE;
	if (currentAxis == xT || currentAxis == yT || currentAxis == zT)
	{
		//rotation around y axis in the positive\negative direction
		if (currentAxis == xT && dy>0)	parameters.v = vec3(DEFAULT_ANGLE, 0, 0)*transformationFactor;
		if (currentAxis == xT && dy<0)	parameters.v = vec3(-1*DEFAULT_ANGLE, 0, 0)*transformationFactor;

		//rotation around y axis in the positive\negative direction
		if (currentAxis == yT && dy>0)	parameters.v = vec3(0, DEFAULT_ANGLE, 0)*transformationFactor;
		if (currentAxis == yT && dy<0)	parameters.v = vec3(0, -1*DEFAULT_ANGLE, 0)*transformationFactor;

		//rotation around z axis in the positive\negative direction
		if (currentAxis == zT && dy>0)	parameters.v = vec3(0, 0, DEFAULT_ANGLE)*transformationFactor;
		if (currentAxis == zT && dy<0)	parameters.v = vec3(0, 0, -1*DEFAULT_ANGLE)*transformationFactor;

		scene->operate(parameters);
	}
	else
	{
		if (x >= width || y >= height || x < 0 || y < 0) return;
		endingPoint = projectToSphere(x, y);
		rotationAxis = cross(startingPoint, endingPoint);
		angle = length(rotationAxis) / length(startingPoint)*length(endingPoint);
		angle = (angle * 180) / M_PI;
		if (isnan(angle)) return;
		parameters.v = rotationAxis;
		parameters.theta = angle;
		scene->operate(parameters);
	}
}

void scale(int dy)
{
	//scaling parameters
	vec3 uniformScalingVector = vec3(1, 1, 1);
	parameters.type = SCALE;
	parameters.v = uniformScalingVector;
	if (dy < 0)
	{
		if (currentAxis == xT) parameters.v[0] *= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == yT) parameters.v[1] *= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == zT) parameters.v[2] *= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == uniformT) parameters.v *= (DEFAULT_SCALING_FACTOR*transformationFactor);
		scene->operate(parameters);
	}
	else
	{
		if (currentAxis == xT) parameters.v[0] /= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == yT) parameters.v[1] /= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == zT) parameters.v[2] /= (DEFAULT_SCALING_FACTOR*transformationFactor);
		if (currentAxis == uniformT) parameters.v /= (DEFAULT_SCALING_FACTOR*transformationFactor);
		scene->operate(parameters);
	}
}

void translate(int x, int y, int dy)
{
	parameters.type = TRANSLATE;
	parameters.v = vec3(0, 0, 0);
	if (currentAxis == xT || currentAxis == yT || currentAxis == zT)
	{
		//translation along x axis in the positive\negative direction
		if (currentAxis == xT && dy>0)	parameters.v = vec3(DEFAULT_TRANSLATION, 0, 0)*transformationFactor;
		if (currentAxis == xT && dy<0)	parameters.v = vec3(-1*DEFAULT_TRANSLATION, 0, 0)*transformationFactor;

		//translation along y axis in the positive\negative direction
		if (currentAxis == yT && dy>0)	parameters.v = vec3(0, DEFAULT_TRANSLATION, 0)*transformationFactor;
		if (currentAxis == yT && dy<0)	parameters.v = vec3(0, -1*DEFAULT_TRANSLATION, 0)*transformationFactor;

		//translation along z axis in the positive\negative direction
		if (currentAxis == zT && dy>0)	parameters.v = vec3(0, 0, DEFAULT_TRANSLATION)*transformationFactor;
		if (currentAxis == zT && dy<0)	parameters.v = vec3(0, 0, -1*DEFAULT_TRANSLATION)*transformationFactor;

		scene->operate(parameters);
	}
	else
	{
		if (x >= width || y >= height || x < 0 || y < 0) return;
		endingPoint = projectToSphere(x, y);
		parameters.v = (endingPoint - startingPoint)*transformationFactor;
		scene->operate(parameters);
	}
}

void motion(int x, int y)
{
	redraw = true;
	// calc difference in mouse movement
	int dx=x-last_x;
	int dy=y-last_y;
	// update last x,y
	last_x=x;
	last_y=y;
	int modifier = glutGetModifiers();
	parameters.frame = currentObjectFrame;

	switch (modifier)
	{
		case GLUT_ACTIVE_CTRL:
			scale(dy);
		break;

		case GLUT_ACTIVE_SHIFT:
			translate(x, y, dy);
		break;

		default:
			rotate(x,y,dy);
		break;
	}
}

void newMenuCallback(int id)
{
	if (id == NEW_MODEL){
		CFileDialog dlg(TRUE, _T(".obj"), NULL, NULL, _T("*.obj|*.*"));
		if (dlg.DoModal() == IDOK)
		{
			std::string s((LPCTSTR)dlg.GetPathName());
			scene->loadOBJModel((LPCTSTR)dlg.GetPathName());
		}
		else{
			return;
		}
	}
	switch (id){
	case NEW_CAMERA:
		scene->createCamera();
		break;
	case NEW_PYRAMID:
		scene->addPyramidMesh();
		break;
	case NEW_DEFAULT_LIGHT:
		scene->addDefaultLight();
		break;
	case NEW_CUSTOM_LIGHT:
		Light l;
		string lightTypeString;
		lightType type;
		CCmdDialog dialogType("pleaes choose a type of light - paralel or point");
		CXyzDialog parameterPos("enter light's position in camera - 3d coords");
		CXyzDialog parameterDirection("enter light's direction in camera - 3d coords");
		CXyzDialog parameterIntensity("enter intensity [0,1]: x = ambient, y = diffuse, z = specular, ");
		CXyzDialog parameterColor("enter the light's color in RGB = xyz FORMAT [0,255]");

		if (dialogType.DoModal() == IDOK){
			lightTypeString = dialogType.GetCmd();
			if (lightTypeString == "paralel"){
				l.type = PARALLEL_LIGHT;
				if (parameterDirection.DoModal() == IDOK){
					l.direction = scene->cameraCoordsToWorld(parameterDirection.GetXYZ());
				}
				else{ return; }
			}
			else if (lightTypeString == "point"){
				l.type = POINT_LIGHT;
				if (parameterPos.DoModal() == IDOK){
					l.position = scene->cameraCoordsToWorld(parameterPos.GetXYZ());
				}
				else{ return; }
			}
			else{
				cout << "invalid light type" << endl;
				return;
			}
		}else{ return; }
		if (parameterIntensity.DoModal() == IDOK){
			vec3 tmpV = parameterIntensity.GetXYZ();
			l.ambientIntensity = tmpV[0];
			l.diffuseIntensity = tmpV[1];
			l.specularIntensity = tmpV[2];
		}else{ return; }
		if (parameterColor.DoModal() == IDOK){
			l.changeColor(parameterColor.GetXYZ());
		}else{ return; }
		scene->addLight(l);
		break;
	}
	redraw = true;
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
			else{
				return;
			}
		}
		scene->selectActiveModel(chosenName);
		redraw = true;
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
		else{
			return;
		}
	}
	transformationFactor = scannedNum;
}

void setCameraPerspective()
{
	ProjectionType projType;
	vec3 parameters;
	CCmdDialog type("pleaes choose projection type: frustrum, ortho or perspective");
	CXyzDialog parameters1("please enter left right and bottom, in that order");
	CXyzDialog parameters2("please enter top near and far in that order");
	CXyzDialog parameters3("please enter z-near and z-far in that order");
	CXyzDialog parameters4("please enter fovy and aspect in that order");
	string projectionType;
	if (type.DoModal() == IDOK)
	{
		projectionType = type.GetCmd();
	}
	else{
		return;
	}
	if (projectionType == "frustrum" || projectionType == "ortho")
	{
		if (parameters1.DoModal() == IDOK)
		{
			parameters = parameters1.GetXYZ();
		}
		else{
			return;
		}
		projection.left = parameters[0];
		projection.right = parameters[1];
		projection.bottom = parameters[2];
		if (parameters2.DoModal() == IDOK)
		{
			parameters = parameters2.GetXYZ();
		}
		else{
			return;
		}
		projection.top = parameters[0];
		projection.zNear = parameters[1];
		projection.zFar = parameters[2];

		if (projectionType == "frustrum")
		{
			projType = FRUSTUM;
		}
		else
		{
			projType = ORTHO;
		}
	}
	else if (projectionType == "perspective")
	{
		if (parameters3.DoModal() == IDOK)
		{
			parameters = parameters3.GetXYZ();
		}
		else{
			return;
		}
		projection.zNear = parameters[0];
		projection.zFar = parameters[1];
		if (parameters4.DoModal() == IDOK)
		{
			parameters = parameters4.GetXYZ();
		}
		else{
			return;
		}
		projection.fovy = parameters[0];
		projection.aspect = parameters[1];
		projType = PERSPECTIVE;
	}
	else
	{
		return;
	}
	scene->setProjection(projType, projection);
	redraw = true;
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
	switch (id){
	case LOOKAT_ACTIVE_MODEL:
		scene->LookAtActiveModel();
		redraw = true;
		break;
	case SET_TRANSFORMATION_STEP:
		setTransformationStep();
		break;
	case SET_CAMERA_PRESPECTIVE:
		setCameraPerspective();
		redraw = true;
		break;
	}
	if (id == SET_MODEL_COLOR || id == SET_LIGHT_COLOR){
		vec3 c;
		CXyzDialog colorMsg("please enter RGB color as [0,255] value per base color");
		if (colorMsg.DoModal() == IDOK)
		{
			c = colorMsg.GetXYZ();
			//check color validation:
			if (c[0] < 0 || c[0] > 255 ||
				c[1] < 0 || c[1] > 255 ||
				c[2] < 0 || c[2] > 255){
				cout << "your color does not exist" << endl;
				return;
			}
		}
		else{ return; }
		(id == SET_MODEL_COLOR) ? scene->changeModelColor(c) : scene->changeLightColor(c);
		redraw = true;
	}
	if (id == SET_LIGHT_DIRECTION){
		vec3 d;
		CXyzDialog dirMsg("please enter current light direction");
		if (dirMsg.DoModal() == IDOK)
		{
			d = dirMsg.GetXYZ();
		}else{ return; }
		scene->changeLightDirection(d);
		redraw = true;
	}
	if (id == SET_LIGHT_INTENSITY){
		CXyzDialog dirMsg("please enter current light's intensity");
		if (dirMsg.DoModal() == IDOK)
		{
			vec3 v = dirMsg.GetXYZ();
			if (v[0] < 0 || v[0] > 1 ||
				v[1] < 0 || v[1] > 1 ||
				v[2] < 0 || v[2] > 1){
				cout << "you have entered an invalid intensity" << endl;
			}
			else{
				scene->changeLightIntensity(v);
			}
		}else{ return; }
		redraw = true;
	}
}

void toggleMenuCallback(int id)
{
	switch (id)
	{
	case BOUNDING_BOX:
		scene->featuresStateSelection(TOGGLE_BOUNDING_BOX);
		redraw = true;
		break;

	case VERTEX_NORMALS:
		scene->featuresStateSelection(TOGGLE_VERTEX_NORMALS);
		redraw = true;
		break;

	case FACE_NORMALS:
		scene->featuresStateSelection(TOGGLE_FACE_NORMALS);
		redraw = true;
		break;

	case CAMERA_RENDERING:
		scene->featuresStateSelection(TOGGLE_CAMERA_RENDERING);
		redraw = true;
		break;
	
	case KEYBOARD_MODE:
		keyboardMode = (keyboardMode == CG1) ? CG2 : CG1;
		break;
	
	case ACTIVE_LIGHT_TYPE:
		CXyzDialog parameterPos("to change to POINT_LIGHT - enter light's position in camera - 3d coords");
		CXyzDialog parameterDirection("to change to PARALLEL_LIGHT - enter light's direction in camera - 3d coords");
		if (scene->getLightType() == POINT_LIGHT){
			if (parameterPos.DoModal() == IDOK){
				scene->changeLightDirection(parameterPos.GetXYZ());
			}
			else{ return; }
		}
		else{
			if (parameterPos.DoModal() == IDOK){
				scene->changeLightPosition(parameterPos.GetXYZ());
			}
			else{ return; }
		}
		scene->toggleActiveLightType();
		break;
	}
}

void initMenu()
{
	int newMenu = glutCreateMenu(newMenuCallback);
	glutAddMenuEntry("Model", NEW_MODEL);
	glutAddMenuEntry("Pyramid", NEW_PYRAMID);
	glutAddMenuEntry("Camera", NEW_CAMERA);
	glutAddMenuEntry("default Light source", NEW_DEFAULT_LIGHT);
	glutAddMenuEntry("customized Light source", NEW_CUSTOM_LIGHT);
	int selectMenu = glutCreateMenu(selectMenuCallback);
	glutAddMenuEntry("Active model", ACTIVE_MODEL);
	int toggleMenu = glutCreateMenu(toggleMenuCallback);
	glutAddMenuEntry("Face normals", FACE_NORMALS);
	glutAddMenuEntry("Vertex normals", VERTEX_NORMALS);
	glutAddMenuEntry("Bounding box", BOUNDING_BOX);
	glutAddMenuEntry("Camera rendering", CAMERA_RENDERING);
	glutAddMenuEntry("keyboard mode", KEYBOARD_MODE);
	glutAddMenuEntry("active light type", ACTIVE_LIGHT_TYPE);
	int toolsMenu = glutCreateMenu(toolsMenuCallback);
	glutAddMenuEntry("LookAt active model", LOOKAT_ACTIVE_MODEL);
	glutAddMenuEntry("Set transformation step", SET_TRANSFORMATION_STEP);
	glutAddMenuEntry("Set camera perspective", SET_CAMERA_PRESPECTIVE);
	glutAddMenuEntry("set a new color for the model", SET_MODEL_COLOR);
	glutAddMenuEntry("set a new color for the current light", SET_LIGHT_COLOR);
	glutAddMenuEntry("set a direction for the current light", SET_LIGHT_DIRECTION);
	glutAddMenuEntry("set intensity for the current light", SET_LIGHT_INTENSITY);
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
	keyboardMode = CG2;
	currentCameraFrame = CAMERA_VIEW;
	currentObjectFrame = WORLD;
	//----------------------------------------------------------------------------
	// Initialize Callbacks

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutMotionFunc ( motion );
	glutReshapeFunc( reshape );
	glutSpecialFunc(special);
	glutMouseWheelFunc( mouseWheel );
	glutIdleFunc(idle);
	initMenu();
	renderer->refresh();
	renderer->SwapBuffers();
	glutMainLoop();
	delete scene;
	delete renderer;
	return 0;
}

CWinApp theApp;

using namespace std;

int main( int argc, char **argv )
{
	DebugMode d = OFF;
	
	if (d == OFF){
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
			my_main(argc, argv);
		}

		return nRetCode;
	}
	/*=======================================
			TEST BELOW					
	=======================================*/
	else{
		mainOverallTest();
		return 0;
	}
}
