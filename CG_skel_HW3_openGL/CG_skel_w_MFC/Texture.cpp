#include "stdafx.h"
#include "Texture.h"

using std::cout;
using std::endl;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/************************************************
*				TEXTURE CLASS				    *
************************************************/
Texture::Texture(const string& texturePath){
	glGenTextures(1, &id);
	bind();
	int width, height, channels;
	//load texture image to an array, width and height will hold its width and height and channels will hold the number of color channels
	stbi_set_flip_vertically_on_load(true);
	unsigned char* textureData = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
	stbi_set_flip_vertically_on_load(false);
	if (textureData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		cout << "failed to load texture from the path: " + texturePath << endl;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unbind();
	stbi_image_free(textureData);
}
Texture::~Texture()	{}
void Texture::destroy(){
	glDeleteTextures(1, &id);
}
void Texture::bind(){
	glBindTexture(GL_TEXTURE_2D, id);
}
void Texture::unbind(){
	glBindTexture(GL_TEXTURE_2D, 0);
}
GLuint Texture::getId(){
	return id;
}

/************************************************
*			ENVIRONMENT TEXTURE CLASS		    *
************************************************/

////////////////////////////////////////////////////
//				 STATIC VARIABLERS   			 //
//////////////////////////////////////////////////

GLfloat Environment::boxVertices[NUMBER_OF_VERTICES * 3] =
{
	// positions          
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f
};


////////////////////////////////////////////////////
//				 PRIVATE FUNCTIONS  			 //
//////////////////////////////////////////////////
vector<string> Environment::generateFacesFilesLocations(){
	vector<string> locs;
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_rt.tga");
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_lf.tga");
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_up.tga");
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_dn.tga");
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_bk.tga");
	locs.push_back("D:\\software\\gitProjects\\ComputerGraphics\\CG_skel_HW3_openGL\\CG_skel_w_MFC\\environmentFiles\\emerald-summit_ft.tga");
	return locs;
}
void Environment::bindWithDraw()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glBindVertexArray(boxVAO);
}
void Environment::unbindAfterDraw()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}
void Environment::createCubeMap(const vector<string>& textureLocations)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	int locationsNum = textureLocations.size();
	int width, height, channels;
	unsigned char *texBuffer = NULL;
	for (GLuint i = 0; i < locationsNum; ++i)
	{
		texBuffer = stbi_load(textureLocations[i].c_str(), &width, &height, &channels, 0);
		if (texBuffer)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texBuffer);
			stbi_image_free(texBuffer);
		}
		else
		{
			std::cout << "failed to load environment texture in position: " << endl << textureLocations[i] << std::endl;
			stbi_image_free(texBuffer);
			cin >> width;
			break;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Environment::initBuffers()
{
	glGenVertexArrays(1, &boxVAO);
	glBindVertexArray(boxVAO);
	glGenBuffers(1, &boxVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, boxVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * NUMBER_OF_VERTICES * 3, boxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

////////////////////////////////////////////////////
//				 PUBLIC FUNCTIONS   			 //
//////////////////////////////////////////////////
Environment::Environment(): 
shader(ShaderProgram("environment.vs", "environment.fs")){
	vector<string> locs = generateFacesFilesLocations();
	createCubeMap(locs);
	initBuffers();
}
Environment::Environment(ShaderProgram &shader) :
shader(shader)
{
	vector<string> locs = generateFacesFilesLocations();
	createCubeMap(locs);
	initBuffers();
}
Environment::Environment(const vector<string>& textureLocations, ShaderProgram shader) :
shader(shader){
	createCubeMap(textureLocations);
	initBuffers();
}
Environment::Environment(const Environment &e){
	id				= e.id;
	boxVAO			= e.boxVAO;
	boxVerticesVBO	= e.boxVerticesVBO;
	shader			= e.shader;
}
void Environment::draw(){
	glDepthMask(GL_FALSE);
	shader.activate();
	bindWithDraw();
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);
	glDisableVertexAttribArray(0);
	unbindAfterDraw();
	shader.deactivate();
	glDepthMask(GL_TRUE);
}
void Environment::bind(){
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
void Environment::unbind(){
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void Environment::destroy(){
	shader.deleteProgram();
	glDeleteVertexArrays(1, &boxVAO);
	glDeleteTextures(1, &id);
}
void Environment::updateViewAndProjection(mat4 view, mat4 projection, vec4 eye){
	shader.setUniform("view", view);
	//shader.setUniform("eye", eye.vec4ToVec3());
	shader.setUniform("projection", projection);
}