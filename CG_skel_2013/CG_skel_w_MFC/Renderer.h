#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

#define DEFAULT_SCREEN_X 512
#define DEFAULT_SCREEN_Y 512
#define DEFAULT_R 255
#define DEFAULT_G 255
#define DEFAULT_B 255

enum drawType{VERTEX, NORMAL};
enum clipResult{IN_BOUNDS, OUT_OF_BOUNDS, CLIPPED};

using namespace std;
class Renderer
{
private:
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	mat4 cameraTransform;
	mat4 projection;
	mat4 objectTransform;
	mat4 normalTransform;
	
	mat4 totalPipline;
	mat4 normalPipeline;

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void resizeBuffers(int chosenWidth, int chosenHeight);
	clipResult clipLine(vec4& endPointA, vec4& endPointB);
	vec2 transformToScreen(vec4 vertex);
	void drawFaceNormals(vec3* vertexPositions, vec3* faceNormals, int vertexPositionsSize);
	void drawVertexNormals(vec3* vertexPositions,vec3* vertexNormals, int vertexSize);
	void drawBoundingBox(vec3* boundingBoxVertices);
	void drawTriangles(vec3* vertexPositions, int vertexPositionsSize);
	void drawLine(const vec2& v0, const vec2& v1);
	void setLineInBuffer(int xMin, int xMax, int yMin, int yMax, int horizontalDirection, int verticalDirection, int swapped, float *m_outBuffer);
	void plotPixel(int x, int y, float* m_outBuffer, vec3 RGB);
	void SetCameraTransform(const mat4& chosenCameraTransform);
	void SetProjection(const mat4& chosenProjection);
	void SetObjectMatrices(const mat4& chosenObjectTransform, const mat3& chosenNormalTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
	void refresh();
};