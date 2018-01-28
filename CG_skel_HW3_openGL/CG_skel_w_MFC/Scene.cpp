#include "stdafx.h"
#include "Scene.h"
#include "Vertex.h"
#include <string>
#include "GL\freeglut.h"
#include "MeshModel.h"

using namespace std;

#define INVALID_INDEX -1

/*===============================================================
				static aux functions:
===============================================================*/
static MeshModel* changeToMeshModel(Model* m){
	return static_cast<MeshModel*>(m);
}
static void initState(){
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/*===============================================================
				scene private functions:
===============================================================*/
void Scene::initPrograms(){
	//must push the data in accordance with Programs enumeration.
	programs.push_back(ShaderProgram("minimal_vshader.glsl"			, "minimal_fshader.glsl"							));	//PROGRAM_MINIMAL
	programs.push_back(ShaderProgram("normalShader.vs"				, "normalShader.gs"				, "normalShader.fs"	));	//PROGRAM_NORMAL
	programs.push_back(ShaderProgram("phongShader.vs"				, "phongShader.fs"									));	//PROGRAM_PHONG
	programs.push_back(ShaderProgram("phongWithEnvironmentMap.vs"	, "phongWithEnvironmentMap.fs"						));	//PROGRAM_PHONG_WITH_ENV
	programs.push_back(ShaderProgram("gouraudShader.vs"				, "gouraudShader.fs"								));	//PROGRAM_GOURAUD
	programs.push_back(ShaderProgram("Texture.vs"					, "Texture.fs"										));	//PROGRAM_TEXTURE
	//TODO: add more shaders' programs.
}
void Scene::initData(){
	lights.push_back(new Light());
	cameras.push_back(new Camera(0));
	camerasModels.push_back(new PrimMeshModel());
	activeModel = INVALID_INDEX;
	activeLight = 0;
	activeCamera = 0;
	shading = FLAT;
	renderCamerasFlag = false;
	environmentDisplayFlag = false;
}
void Scene::handleMeshModelFrame(OperateParams &p){
	if (activeModel == INVALID_INDEX){ return; }
	MeshModel* m = changeToMeshModel(models[activeModel]);
	m->frameActionSet((p.frame == MODEL) ? OBJECT_ACTION : WORLD_ACTION);
	switch (p.type){
	case TRANSLATE: m->translate(p.v);
		break;
	case ROTATE:m->rotateXYZ(p.v);
		break;
	case SCALE: m->scale(p.v);
		break;
	}
}
void Scene::handleCameraFrame(OperateParams &p){
	mat4 A, invA;
	switch (p.type){
	case TRANSLATE: 
		A = Translate(-p.v);
		invA = Translate(p.v);
		break;
	case ROTATE:	
		A = RotateZ(p.v[Z]) * RotateY(p.v[Y]) * RotateX(p.v[X]);
		invA = RotateX(-p.v[X]) * RotateY(-p.v[Y]) * RotateZ(-p.v[Z]);
		cameras[activeCamera]->updateEnvironmentMat(A);
		break;
	case SCALE:		
		cameras[activeCamera]->zoom(p.floatData);
		return;
	}
	cameras[activeCamera]->setTransformation(A, invA);
}
void Scene::updateLights(){
	int size = programs.size();
	for (int i = 0; i < size; ++i){
		programs[i].setUniform(lights);
	}
}
/*===============================================================
				scene public functions:
===============================================================*/
Scene::Scene(){
	initPrograms();
	initState();
	initData();
}
Scene::~Scene(){
	for (int i = 0; i < models.size(); ++i){
		delete(models[i]);
		models[i] = NULL;
	}
	for (int i = 0; i < lights.size(); ++i){
		delete(lights[i]);
		lights[i] = NULL;
	}
	for (int i = 0; i < cameras.size(); ++i){
		delete(cameras[i]);
		cameras[i] = NULL;
	}
	for (int i = 0; i < programs.size(); ++i){
		programs[i].deleteProgram();
	}
}
void Scene::loadOBJModel(string fileName)
{
	models.push_back(new MeshModel(fileName));
	activeModel = models.size() - 1;
	LookAtActiveModel();
}
void Scene::draw(){
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	cameras[activeCamera]->updatePrograms(programs);
	updateLights();
	if (environmentDisplayFlag){
		cameras[activeCamera]->updateEvironment(environment);
		environment.draw();
		environment.bind();
	}
	for (int i = 0; i < models.size(); ++i){
		changeToMeshModel(models[i])->draw(programs);
	}
	if (renderCamerasFlag){
		for (int i = 0; i < camerasModels.size(); ++i){
			Model *m = camerasModels[i];
			static_cast<PrimMeshModel*>(m)->setWorldTransform(cameras[i]->getCameraToWorld());
			if (i != activeCamera){
				m->draw(programs);
			}
		}
	}
	if (environmentDisplayFlag){
		environment.unbind();
	}
	glutSwapBuffers();
}
void Scene::createCamera(){
	cameras.push_back(new Camera(cameras.size()));
	camerasModels.push_back(new PrimMeshModel);
	LookAtCameraPyramid(ORTHO, cameras.size() - 1);
}
void Scene::setActiveModel(int i){
	if (models.size() > i){
		activeModel = i;
		cout << "object num " << i << " was chosen" << endl;
	}
	else{
		cout << "you've entered invalid model index, only " << models.size() << "models ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << models.size() - 1 << "]" << endl;
	}
}
void Scene::setActiveLight(int i){
	if (lights.size() > i){
		activeLight = i;
	}
	else{
		cout << "you've entered invalid light index, only " << lights.size() << "lights ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << lights.size() - 1 << "]" << endl;
	}
}
void Scene::setActiveCamera(int i){
	if (cameras.size() > i){
		activeCamera = i;
	}
	else{
		cout << "you've entered invalid camera index, only " << cameras.size() << "cameras ";
		cout << "are in the system" << endl;
		cout << "please enter int value in the range [0, " << cameras.size() - 1 << "]" << endl;
	}
}
void Scene::resetActiveModelTransformations(){
	if (activeModel != INVALID_INDEX){
		changeToMeshModel(models[activeModel])->resetTransformations();
	}
	LookAtActiveModel(cameras[activeCamera]->getType());
}
void Scene::operate(OperateParams &p){
	switch (p.frame){
	case MODEL:	handleMeshModelFrame(p); break;
	case WORLD:	handleMeshModelFrame(p); break;
	case CAMERA:handleCameraFrame(p); break;
	}
}
void Scene::featuresStateSelection(ActivationToggleElement e){
	if (e == TOGGLE_CAMERA_RENDERING){
		renderCamerasFlag = renderCamerasFlag ? false : true;
	}
	else{
		if (activeModel != INVALID_INDEX){
			changeToMeshModel(models[activeModel])->featuresStateToggle(e);
		}
	}
}
void Scene::addPyramidMesh(){
	models.push_back(new PrimMeshModel());
	activeModel = models.size() - 1;
	LookAtActiveModel();
}
void Scene::setProjection(ProjectionType &type, ProjectionParams &p){
	switch (type){
	case ORTHO:	cameras[activeCamera]->Ortho(p); break;
	case FRUSTUM: cameras[activeCamera]->Frustum(p); break;
	case PERSPECTIVE: cameras[activeCamera]->Perspective(p); break;
	}
}
void Scene::LookAtActiveModel(){
	LookAtActiveModel(ORTHO);
}
void Scene::LookAtActiveModel(ProjectionType pType){
	if (activeModel == INVALID_INDEX){ return; }
	MeshModel* m = changeToMeshModel(models[activeModel]);
	vec3 meshCenter = m->getCenterOfMass();
	vec3 deltas = m->getVolume();
	GLfloat dx = deltas[0];
	GLfloat dy = deltas[1];
	GLfloat dz = deltas[2];
	if (dx < 0 || dy < 0 || dz < 0){
		cout << "error with LookAtActiveModel" << endl;
		return;
	}
	GLfloat dMax = (dx < dy) ? (dy < dz ? dz : dy) : (dx < dz ? dz : dx);
	vec4 eye(meshCenter);
	eye[2] += 5 * dMax;	//set the x value of the eye to be far enough from the box
	vec4 at(meshCenter);	//look at center of mesh
	vec4 up(0, 1, 0, 1);	//up is set to z axis

	cameras[activeCamera]->LookAt(eye, at, up);

	//set needed projection params:
	ProjectionParams p;
	if (pType == ORTHO){
		p.left = -(dMax * 2) / 3;
		p.right = (dMax * 2) / 3;
		p.bottom = -(dMax * 2) / 3;
		p.top = (dMax * 2) / 3;
		p.zNear = dMax * 4;
		p.zFar = dMax * 6;
	}
	else if (pType == FRUSTUM){
		p.left = -(dMax * 2) / 3;
		p.right = (dMax * 2) / 3;
		p.bottom = -(dMax * 2) / 3;
		p.top = (dMax * 2) / 3;
		p.zNear = dMax * 4;
		p.zFar = dMax * 6;
	}else{
		p.zNear = dMax * 4;
		p.zFar = dMax * 6;
		p.fovy = 15;
		p.aspect = 1;
	}

	switch (pType){
	case ORTHO:			
		cameras[activeCamera]->Ortho(p);
		break;
	case FRUSTUM:		
		cameras[activeCamera]->Frustum(p);
		break;
	case PERSPECTIVE:	
		cameras[activeCamera]->Perspective(p);
		break;
	}
}
void Scene::LookAtCameraPyramid(ProjectionType pType, int cameraIndex){
	if (cameraIndex < 0 || cameraIndex >= camerasModels.size()){ return; }
	PrimMeshModel* m = static_cast<PrimMeshModel*>(camerasModels[cameraIndex]);
	vec3 meshCenter = m->getCenterOfMass();
	vec3 deltas = m->getVolume();
	GLfloat dx = deltas[0];
	GLfloat dy = deltas[1];
	GLfloat dz = deltas[2];
	if (dx < 0 || dy < 0 || dz < 0){
		cout << "error with LookAtActiveCamera" << endl;
		return;
	}
	GLfloat dMax = (dx < dy) ? (dy < dz ? dz : dy) : (dx < dz ? dz : dx);
	vec4 eye(meshCenter);
	eye[2] += 5 * dMax;	//set the x value of the eye to be far enough from the box
	vec4 at(meshCenter);	//look at center of mesh
	vec4 up(0, 1, 0, 0);	//up is set to z axis

	cameras[activeCamera]->LookAt(eye, at, up);

	//set needed projection params:
	ProjectionParams p;
	if (pType == ORTHO){
		p.left = -dMax * 2;
		p.right = dMax * 2;
		p.bottom = -dMax * 2;
		p.top = dMax * 2;
		p.zNear = dMax * 1;
		p.zFar = dMax * 10;
	}
	else if (pType == FRUSTUM){
		p.left = -(dMax * 2) / 3;
		p.right = (dMax * 2) / 3;
		p.bottom = -(dMax * 2) / 3;
		p.top = (dMax * 2) / 3;
		p.zNear = dMax * 4;
		p.zFar = dMax * 6;
	}
	else{
		p.zNear = dMax * 4;
		p.zFar = dMax * 6;
		p.fovy = 15;
		p.aspect = 1;
	}

	switch (pType){
	case ORTHO:
		cameras[activeCamera]->Ortho(p);
		break;
	case FRUSTUM:
		cameras[activeCamera]->Frustum(p);
		break;
	case PERSPECTIVE:
		cameras[activeCamera]->Perspective(p);
		break;
	}
}
void Scene::changeProjectionRatio(GLfloat widthRatioChange, GLfloat heightRatioChage){
	cameras[activeCamera]->changeProjectionRatio(widthRatioChange, heightRatioChage);
}
void Scene::toggleFogMode(){
	static bool flag = false;
	flag = !flag;
	programs[PROGRAM_PHONG].setUniform("fogFlag",flag);
}
void Scene::toggleAliasingMode(){
	static bool state = false;
	if (state){
		state = false;
		glDisable(GL_POLYGON_SMOOTH);
	}
	else{
		state = true;
		glEnable(GL_POLYGON_SMOOTH);
	}
}
void Scene::toggleEnvironmentMapping(){
	environmentDisplayFlag = !environmentDisplayFlag;
	if (environmentDisplayFlag){
		for (int i = 0; i < models.size(); ++i){
			changeToMeshModel(models[i])->setDisplayMode(DM_PHONG_WITH_ENVIRONMENT); //TODO: RETHINK
		}
	}
	else{
		for (int i = 0; i < models.size(); ++i){
			changeToMeshModel(models[i])->setDisplayMode(DM_PHONG);//TODO: RETHINK
		}
	}
}
void Scene::toggleToonShading(){
	static bool toonShadingFlag = false;
	toonShadingFlag = !toonShadingFlag;
	DisplayMode mode = DM_PHONG;
	if (toonShadingFlag){
		mode = DM_TOON_SHADING;
	}
	changeToMeshModel(models[activeModel])->setDisplayMode(mode);
}
void Scene::setShading(ShadingMethod s){
	shading = s;
	DisplayMode mode;
	switch (s){
	case SILHOUETTE:
		mode = DM_FILLED_SILHOUETTE;
		break;
	case FLAT:
		mode = DM_FLAT;
		break;
	case GOURAUD:
		mode = DM_GOURAUD;
		break;
	case PHONG:
		mode = DM_PHONG;
		break;
	}
	for (int i = 0; i < models.size(); ++i){
		changeToMeshModel(models[activeModel])->setDisplayMode(mode);
	}
}
void Scene::addDefaultLight(){
	Light l;
	addLight(l);
}
void Scene::addLight(Light l){
	lights.push_back(new Light(l));
	l.print();
}
LightType Scene::getLightType(){
	return lights[activeLight]->type;
}
vec3 Scene::cameraCoordsToWorld(vec3 v){
	return cameras[activeCamera]->getWorldVector(v);
}
void Scene::toggleActiveLightType(){
	lights[activeLight]->type = (lights[activeLight]->type == POINT_LIGHT) ? PARALLEL_LIGHT : POINT_LIGHT;
}
void Scene::activeLightIncrementStats(LightStat s){
	lights[activeLight]->incrementDefaultIntensity(s);
}
void Scene::activeLightDecrementStats(LightStat s){
	lights[activeLight]->decrementDefaultIntensity(s);
}
void Scene::changeLightColor(vec3 c){
	lights[activeLight]->changeColor(c);
}
void Scene::changeLightDirPos(vec3 dir){
	if (lights[activeLight]->type == POINT_LIGHT){
		lights[activeLight]->position = cameras[activeCamera]->getWorldVector(dir);
	}
	else{
		lights[activeLight]->direction = cameras[activeCamera]->getWorldVector(dir);
	}
}
void Scene::changeLightDirection(vec3 dir){
	lights[activeLight]->direction = cameras[activeCamera]->getWorldVector(dir);
}
void Scene::changeLightPosition(vec3 pos){
	lights[activeLight]->position = cameras[activeCamera]->getWorldVector(pos);
}
void Scene::addToLightPosition(vec3 pos){
	if (lights[activeLight]->type != POINT_LIGHT){
		cout << "current light is parallel - you will not see any diffrence.." << endl;
	}
	lights[activeLight]->position += cameras[activeCamera]->getWorldVector(pos);
}
void Scene::changeLightIntensity(vec3 v){
	lights[activeLight]->changeIntensity(v);
}
void Scene::changeModelColor(vec3 c){
	if (models.empty()){ return; }
	changeToMeshModel(models[activeModel])->setUniformColor(c);
}
void Scene::setActiveModelMaterial(vec3 emissive, vec3 ambient, vec3 diffuse, vec3 specular){
	if (models.empty()){ return; }
	changeToMeshModel(models[activeModel])->setUniformColor(emissive, ambient, diffuse, specular);
}
void Scene::printActiveModelMaterial(){
	if (models.empty()){ return; }
	changeToMeshModel(models[activeModel])->printUniformMateral();
}
void Scene::printActiveLight(){
	lights[activeLight]->print();
}
void Scene::setNonUniformMaterialForActiveModel(){
	changeToMeshModel(models[activeModel])->setNonUniformMaterial();
}
void Scene::setUniformMaterialForActiveModel(){
	Material m;
	changeToMeshModel(models[activeModel])->setUniformMaterial(m);
}
void Scene::setDisplayMode(DisplayMode mode){
	if (models.empty()){ return; }
	changeToMeshModel(models[activeModel])->setDisplayMode(mode);
}
void Scene::resetPrograms(){
	programs.clear();
	initPrograms();
}
void Scene::animate(vec3 rotationVec, vec3 translationVec, vec3 colorVec){
	int iterationNumber			= 200;
	float iterationFraction		= ((float)1 / (float)iterationNumber);
	vec3 rotationFracture		= rotationVec * iterationFraction;
	vec3 translationFracture	= translationVec * iterationFraction;
	vec3 originColor = 255 * changeToMeshModel(models[activeModel])->getColor();
	vec3 colorFracture = iterationFraction * (colorVec - originColor);
	vec3 currentColor;
	for (int i = 0; i < iterationNumber; ++i){
		changeToMeshModel(models[activeModel])->rotateXYZ(rotationFracture);
		changeToMeshModel(models[activeModel])->translate(translationFracture);
		currentColor = originColor + colorFracture*i;
		clamp(currentColor, 0.0, 255.0);
		changeToMeshModel(models[activeModel])->setUniformColor(currentColor);
		draw();
	}
}
bool Scene::doesActiveModelNeedTextureFile(){
	bool result = false;
	if (models.size() > 0){
		result = changeToMeshModel(models[activeModel])->isTexturePresent();
	}
	return result;
}
void Scene::loadTextureToActiveModel(string fileName){
	changeToMeshModel(models[activeModel])->setTextureFile(fileName);
}
/*===============================================================
							demos:
===============================================================*/
void Scene::drawDemo(){
	changeToMeshModel(models[activeModel])->draw(programs);
	glutSwapBuffers();
	int x;
	cin >> x;
}
void Scene::drawDemo2(){
	const int vNum = 3;
	vec4 v1(-0.1, -0.1f, 0.0f, 1.0f);
	vec4 v2(0.1f, -0.1f, 0.0f, 1.0f);
	vec4 v3(0.0f, 0.1f, 0.0f, 1.0f);
	
	Vertex u1(v1, vec3(1, 0, 0));
	Vertex u2(v2, vec3(1, 0, 0));
	Vertex u3(v3, vec3(0, 0, 1));
	Vertex vertices[vNum] = { u1, u2, u3 };

	const int pSize = 7 * vNum;
	GLfloat data[pSize];
	for (int i = 0; i < vNum; ++i){
		vertices[i].putInBuffer(data, i * 7);
	}

	GLuint program = InitShader("minimal_vshader.glsl",
		"minimal_fshader.glsl");

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	GLuint loc = glGetAttribLocation(program, "vPosition");
	GLuint loc2 = glGetAttribLocation(program, "vColor");

	glEnableVertexAttribArray(loc);
	glEnableVertexAttribArray(loc2);

	glVertexAttribPointer(loc,4,GL_FLOAT,GL_FALSE,7*sizeof(float),0);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(4*sizeof(float)));

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, vNum);
	glFlush();
	glutSwapBuffers();
	
	glDisableVertexAttribArray(loc);
	glDisableVertexAttribArray(loc2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void Scene::drawDemo3()
{
	const int pnum = 3;
	static const GLfloat points[pnum][4] = {
		{-0.1, -0.1f, 0.0f,1.0f},
		{0.1f, -0.1f, 0.0f,1.0f},
		{0.0f,  0.1f, 0.0f,1.0f}
	};

	GLuint program = InitShader( "minimal_vshader.glsl", 
					  "minimal_fshader.glsl" );

	glUseProgram(program);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);


	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_LINE_LOOP, 0, pnum);
	glFlush();
	glutSwapBuffers();
}
