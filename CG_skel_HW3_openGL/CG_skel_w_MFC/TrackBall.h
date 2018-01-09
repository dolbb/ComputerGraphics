#pragma once
#include "vec.h"
class TrackBall
{

public:

	/**
	*	PUBLIC TRACK BALL FUNCTIONS
	*/

	TrackBall(int width, int height);
	~TrackBall();
	void setStart(int x, int y);
	void setEnd(int x, int y);
	vec3 getRotationAxis();
	float getRotationAngle();
	vec3 getTranslation();
	vec3 getNonUniformScaling();
	vec3 getUniformScaling();
	void resetPoints();
	void setViewport(int chosenWidth, int chosenHeight);

private:

	/**
	*	PRIVATE TRACK BALL VARIABLES
	*/

	//endingPoint - startingPoint is the vector we are trying to interpert to a transformation
	vec3 startingPoint;
	vec3 endingPoint;

	//viewport width and height
	int width;
	int	height;

	static float translationFactor;
	static float scalingFactor;

	/**
	*	PRIVATE TRACK BALL FUNCTIONS
	*/

	vec3 projectToSphere(int x, int y);
};

