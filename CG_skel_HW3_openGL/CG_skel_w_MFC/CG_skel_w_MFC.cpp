// CG_skel_w_MFC.cpp : Defines the entry point for the console application.
//


//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

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
#include <string>
#include "InputDialog.h"

#define BUFFER_OFFSET( offset )   ((GLvoid*) (offset))

#define FILE_OPEN 1
#define MAIN_DEMO 1
#define MAIN_ABOUT 2

#define DEFAULT_WIDTH 512
#define DEFAULT_HEIGHT 512

#define DEFAULT_ANGLE 30.0
#define DEFAULT_ZOOM_FACTOR 1.05

enum DebugMode{ 
	ON, 
	OFF 
};
enum MainMenuIdentifier{ 
	DEMO 
};
enum NewMenuIdentifier{ 
	NEW_MODEL, 
	NEW_CAMERA, 
	NEW_PYRAMID, 
	NEW_DEFAULT_LIGHT,
	NEW_CUSTOM_LIGHT 
};
enum ToolsMenuIdentifier{ 
	LOOKAT_ACTIVE_MODEL 
};
enum SetMenuIdentifier{
	SET_CAMERA_PRESPECTIVE, 
	SET_MODEL_GENERAL_COLOR, 
	SET_NON_UNIFORM_MODEL_MATERIAL, 
	SET_UNIFORM_MODEL_MATERIAL, 
	SET_LIGHT_COLOR, 
	SET_LIGHT_DIRECTION, 
	SET_LIGHT_INTENSITY 
};
enum ToggleMenuIdentifier{ 
	FACE_NORMALS, 
	VERTEX_NORMALS, 
	BOUNDING_BOX, 
	CAMERA_RENDERING, 
	ACTIVE_LIGHT_TYPE, 
	ANTI_ALIASING, 
	FOG,
	BLOOM, 
	BLUR 
};
//enum numOfFrames{ OBJECT_FRAMES = 2, CAMERA_FRAMES = 2 };

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
float translationStep = 1.0;
Frames currentObjectFrame = WORLD;
OperateParams operationParams;
ProjectionParams projection;


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
void keyboard(unsigned char key, int x, int y){
	if (key >= '0' && key <= '2')
	{
		int index = key - '0';
		scene->setActiveLight(index);
	}
	else if (key >= '3' && key <= '5'){
		int index = key - '3';
		scene->setActiveCamera(index);
	}
	else if (key >= '6' && key <= '9'){
		int index = key - '6';
		scene->setActiveModel(index);
	}
	OperateParams p;
	p.frame = currentObjectFrame;
	p.type = ROTATE;

	switch (key)
	{
		//ESC
	case 033:
		exit(EXIT_SUCCESS);
		break;
		//change light spec (inc \ dec) in a specific aspect:
	case 'r':
		scene->activeLightIncrementStats(AMBIENT);
		cout << "current light was incremented in its AMBIENT field" << endl;
		break;
	case 'f':
		scene->activeLightDecrementStats(AMBIENT);
		cout << "current light was decremented in its AMBIENT field" << endl;
		break;
	case 't':
		scene->activeLightIncrementStats(DIFFUSE);
		cout << "current light was incremented in its DIFFUSE field" << endl;
		break;
	case 'g':
		scene->activeLightDecrementStats(DIFFUSE);
		cout << "current light was decremented in its DIFFUSE field" << endl;
		break;
	case 'y':
		scene->activeLightIncrementStats(SPECULAR);
		cout << "current light was incremented in its SPECULAR field" << endl;
		break;
	case 'h':
		scene->activeLightDecrementStats(SPECULAR);
		cout << "current light was decremented in its SPECULAR field" << endl;
		break;
		/*object transform operations:*/
	case 'w':
		p.v = vec3(DEFAULT_ANGLE, 0, 0);
		scene->operate(p);
		break;
	case 's':
		p.v = vec3(-DEFAULT_ANGLE, 0, 0);
		scene->operate(p);
		break;
	case 'd':
		p.v = vec3(0, -DEFAULT_ANGLE, 0);
		scene->operate(p);
		break;
	case 'a':
		p.v = vec3(0, DEFAULT_ANGLE, 0);
		scene->operate(p);
		break;
	case 'e':
		p.type = TRANSLATE;
		p.v = vec3(-translationStep / 100.0, 0, 0);
		scene->operate(p);
		break;
	case 'q':
		p.type = TRANSLATE;
		p.v = vec3(translationStep / 100.0, 0, 0);
		scene->operate(p);
		break;
	case 'z':
		p.type = TRANSLATE;
		p.v = vec3(0, 0, -translationStep / 100.0);
		scene->operate(p);
		cout << "model sent back" << endl;
		break;
	case 'x':
		p.type = TRANSLATE;
		p.v = vec3(0, 0, translationStep / 100.0);
		scene->operate(p);
		cout << "model sent front" << endl;
		break;
	case 'l':
		scene->printActiveLight();
		break;
	case 'm':
		cout << "current object transformation frame: ";
		if (currentObjectFrame == MODEL){
			currentObjectFrame = WORLD;
			cout << "world" << endl;
		}
		else{
			currentObjectFrame = MODEL;
			cout << "model" << endl;
		}
		break;
	}
	redraw = true;
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
void special(int key, int x, int y)
{
	int modifier = glutGetModifiers();
	operationParams.frame = CAMERA;
	operationParams.type = ROTATE;
	if (modifier == GLUT_ACTIVE_SHIFT)
	{
		operationParams.type = TRANSLATE;
	}
	switch (key)
	{
	case GLUT_KEY_UP:
		//set translation parameters
		if (operationParams.type == TRANSLATE){
			operationParams.v = vec3(0, 1, 0) * translationStep;
		}
		//set rotation parameters
		else{
			operationParams.v = vec3(1, 0, 0) * DEFAULT_ANGLE;
		}
		break;

	case GLUT_KEY_DOWN:
		//set translation parameters
		if (operationParams.type == TRANSLATE){
			operationParams.v = vec3(0, -1, 0)*translationStep;
		}
		//set rotation parameters
		else{
			operationParams.v = vec3(-1, 0, 0)*DEFAULT_ANGLE;
		}
		break;

	case GLUT_KEY_LEFT:
		//set translation parameters
		if (operationParams.type == TRANSLATE){
			operationParams.v = vec3(-1, 0, 0)*translationStep;
		}
		//set rotation parameters
		else{
			operationParams.v = vec3(0, 1, 0)*DEFAULT_ANGLE;
		}
		break;

	case GLUT_KEY_RIGHT:
		//set translation parameters
		if (operationParams.type == TRANSLATE){
			operationParams.v = vec3(1, 0, 0)*translationStep;
		}
		//set rotation parameters
		else{
			operationParams.v = vec3(0, -1, 0)*DEFAULT_ANGLE;
		}
		break;
	}
	scene->operate(operationParams);
	redraw = true;
}
void mouseWheel(int wheel, int direction, int x, int y)
{
	operationParams.frame = CAMERA;
	operationParams.type = SCALE;
	if (direction > 0)
	{
		operationParams.floatData = DEFAULT_ZOOM_FACTOR;
	}
	else
	{
		operationParams.floatData = 1 / DEFAULT_ZOOM_FACTOR;
	}
	scene->operate(operationParams);
	redraw = true;
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
		LightType type;
		CCmdDialog dialogType("pleaes choose a type of light - parallel(0) or point(1)");
		CXyzDialog parameterPos("enter light's position in camera - 3d coords");
		CXyzDialog parameterDirection("enter light's direction in camera - 3d coords");
		CXyzDialog parameterIntensity("enter intensity [0,1]: x = ambient, y = diffuse, z = specular, ");
		CXyzDialog parameterColor("enter the light's color in RGB = xyz FORMAT [0,255]");

		if (dialogType.DoModal() == IDOK){
			lightTypeString = dialogType.GetCmd();
			if (lightTypeString == "parallel" || lightTypeString == "0"){
				l.type = PARALLEL_LIGHT;
				if (parameterDirection.DoModal() == IDOK){
					l.direction = scene->cameraCoordsToWorld(parameterDirection.GetXYZ());
				}
			}
			else if (lightTypeString == "point" || lightTypeString == "1"){
				l.type = POINT_LIGHT;
				if (parameterPos.DoModal() == IDOK){
					l.position = scene->cameraCoordsToWorld(parameterPos.GetXYZ());
				}
				else{
					l.position = scene->cameraCoordsToWorld(vec3(0, 2, 1));
				}
			}
			else{
				cout << "invalid light type" << endl;
				return;
			}
		}
		else{ return; }
		if (parameterIntensity.DoModal() == IDOK){
			vec3 tmpV = parameterIntensity.GetXYZ();
			l.ambientIntensityScalar = tmpV[0];
			l.diffuseIntensityScalar = tmpV[1];
			l.specularIntensityScalar = tmpV[2];
		}
		else{
			l.ambientIntensityScalar = 0.1;
			l.diffuseIntensityScalar = 0.5;
			l.specularIntensityScalar = 0.8;
		}
		if (parameterColor.DoModal() == IDOK){
			l.changeColor(parameterColor.GetXYZ());
		}
		else{
			l.changeColor(vec3(255, 255, 255));
		}
		scene->addLight(l);
		break;
	}
	redraw = true;
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

void shadingMenuCallback(int id){
	scene->setShading(static_cast<ShadingMethod>(id));
	switch (id){
	case FLAT:
		cout << "changed to FLAT mode" << endl;
		break;
	case GOURAUD:
		cout << "changed to GOURAUD mode" << endl;
		break;
	case PHONG:
		cout << "changed to PHONG mode" << endl;
		break;
	}
	redraw = true;
}
void setMenuCallback(int id){
	switch (id){
	case SET_CAMERA_PRESPECTIVE:
		setCameraPerspective();
		redraw = true;
		break;
	case SET_NON_UNIFORM_MODEL_MATERIAL:
		scene->setNonUniformMaterialForActiveModel();
		redraw = true;
		break;
	}

	if (id == SET_UNIFORM_MODEL_MATERIAL){
		vec3 emissive;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;

		CXyzDialog emissiveMsg("please enter [0,1] values for emissive");
		CXyzDialog ambientMsg("please enter [0,1] values for ambient");
		CXyzDialog diffuseMsg("please enter [0,1] values for diffuse");
		CXyzDialog specularMsg("please enter [0,1] values for specular");

		if (emissiveMsg.DoModal() == IDOK){ emissive = emissiveMsg.GetXYZ(); }
		else{ return; }
		if (ambientMsg.DoModal() == IDOK){ ambient = ambientMsg.GetXYZ(); }
		else{ return; }
		if (diffuseMsg.DoModal() == IDOK){ diffuse = diffuseMsg.GetXYZ(); }
		else{ return; }
		if (specularMsg.DoModal() == IDOK){ specular = specularMsg.GetXYZ(); }
		else{ return; }

		for (int i = 0; i < 3; ++i){
			if (emissive[i] < 0 || emissive[i]	> 1){ cout << "invalid emissive arguments  = " << emissive << endl; return; }
			if (ambient[i]	< 0 || ambient[i]	> 1){ cout << "invalid ambient arguments   = " << ambient << endl; return; }
			if (diffuse[i]	< 0 || diffuse[i]	> 1){ cout << "invalid diffuse arguments   = " << diffuse << endl; return; }
			if (specular[i]	< 0 || specular[i]	> 1){ cout << "invalid specular arguments  = " << specular << endl; return; }
		}
		scene->setActiveModelMaterial(emissive, ambient, diffuse, specular);
		cout << "new uniform material for active model is:" << endl;
		scene->printActiveModelMaterial();
		redraw = true;
	}
	if (id == SET_MODEL_GENERAL_COLOR || id == SET_LIGHT_COLOR){
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
		(id == SET_MODEL_GENERAL_COLOR) ? scene->changeModelColor(c) : scene->changeLightColor(c);
		redraw = true;
	}
	if (id == SET_LIGHT_DIRECTION){
		vec3 d;
		CXyzDialog dirMsg("please enter current light direction//position");
		if (dirMsg.DoModal() == IDOK)
		{
			d = dirMsg.GetXYZ();
		}
		else{ return; }
		scene->changeLightDirPos(d);
		cout << "your light direction//position was changed" << endl;
		redraw = true;
	}
	if (id == SET_LIGHT_INTENSITY){
		CXyzDialog dirMsg("please enter current light's intensity");
		vec3 v;
		if (dirMsg.DoModal() == IDOK)
		{
			v = dirMsg.GetXYZ();
			if (v[0] < 0 || v[0] > 1 ||
				v[1] < 0 || v[1] > 1 ||
				v[2] < 0 || v[2] > 1){
				cout << "you have entered an invalid intensity" << endl;
			}
			else{
				scene->changeLightIntensity(v);
				cout << "light intensity is now: (" << v[0] << "," << v[1] << "," << v[2] << ")" << endl;
			}
		}
		else{ return; }
		redraw = true;
	}
}
void toggleMenuCallback(int id){
	CXyzDialog parameterPos("to change to POINT_LIGHT - enter light's position in camera - 3d coords");
	CXyzDialog parameterDirection("to change to PARALLEL_LIGHT - enter light's direction in camera - 3d coords");
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
	case ACTIVE_LIGHT_TYPE:
		if (scene->getLightType() == POINT_LIGHT){
			if (parameterDirection.DoModal() == IDOK){
				scene->changeLightDirection(parameterDirection.GetXYZ());
				cout << "current light is now PARALLEL type" << endl;
			}
			else{ return; }
		}
		else{
			if (parameterPos.DoModal() == IDOK){
				scene->changeLightPosition(parameterPos.GetXYZ());
				cout << "current light is now POINT type" << endl;
			}
			else{ return; }
		}
		scene->toggleActiveLightType();
		redraw = true;
		break;
	case ANTI_ALIASING:
		scene->toggleAliasingMode();
		cout << "anti-aliasing toggled" << endl;
		redraw = true;
		break;
	case FOG:
		scene->toggleFogMode();
		cout << "fog toggled" << endl;
		redraw = true;
		break;
	case BLOOM:
		scene->toggleBloomMode();
		cout << "bloom toggled" << endl;
		redraw = true;
		break;
	case BLUR:
		scene->toggleBlurMode();
		cout << "blur toggled" << endl;
		redraw = true;
		break;
	}
}
void toolsMenuCallback(int id)
{
	switch (id){
	case LOOKAT_ACTIVE_MODEL:
		scene->LookAtActiveModel();
		redraw = true;
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
	int shadingMenu = glutCreateMenu(shadingMenuCallback);
	glutAddMenuEntry("Flat", FLAT);
	glutAddMenuEntry("Gouraud", GOURAUD);
	glutAddMenuEntry("Phong", PHONG);
	int setMenu = glutCreateMenu(setMenuCallback);
	glutAddMenuEntry("Set camera perspective", SET_CAMERA_PRESPECTIVE);
	glutAddMenuEntry("set a NON UNIFORM material for model", SET_NON_UNIFORM_MODEL_MATERIAL);
	glutAddMenuEntry("set a UNIFORM material for model", SET_UNIFORM_MODEL_MATERIAL);
	glutAddMenuEntry("set a new general color for the model", SET_MODEL_GENERAL_COLOR);
	glutAddMenuEntry("set a new color for the current light", SET_LIGHT_COLOR);
	glutAddMenuEntry("set a direction\position for the current light", SET_LIGHT_DIRECTION);
	glutAddMenuEntry("set intensity for the current light", SET_LIGHT_INTENSITY);
	int toggleMenu = glutCreateMenu(toggleMenuCallback);
	glutAddMenuEntry("Face normals", FACE_NORMALS);
	glutAddMenuEntry("Vertex normals", VERTEX_NORMALS);
	glutAddMenuEntry("Bounding box", BOUNDING_BOX);
	glutAddMenuEntry("Camera rendering", CAMERA_RENDERING);
	glutAddMenuEntry("active light type", ACTIVE_LIGHT_TYPE);
	glutAddMenuEntry("Anti-Aliasing", ANTI_ALIASING);
	glutAddMenuEntry("fog", FOG);
	glutAddMenuEntry("bloom", BLOOM);
	glutAddMenuEntry("blur", BLUR);
	int toolsMenu = glutCreateMenu(toolsMenuCallback);
	glutAddMenuEntry("LookAt active model", LOOKAT_ACTIVE_MODEL);
	glutAddSubMenu("set elements", setMenu);
	glutAddSubMenu("Shading method", shadingMenu);
	glutAddSubMenu("Toggle", toggleMenu);
	glutCreateMenu(mainMenu);
	glutAddSubMenu("New", newMenu);
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
	glutSpecialFunc(special);
	glutMouseWheelFunc(mouseWheel);
	
	initMenu();

	glutMainLoop();
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
