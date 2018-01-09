#include "stdafx.h"
#include "TrackBall.h"

float TrackBall::defaultTranslationStep = 5.0;

TrackBall::TrackBall(int width, int height) :width(width), height(height), startingPoint(0, 0), endingPoint(0.0)	{}

TrackBall::~TrackBall()		{}

void TrackBall::setStart(int x, int y)
{
	startingPoint = projectToSphere(x, y);
}

void TrackBall::setEnd(int x, int y)
{
	endingPoint = projectToSphere(x, y);
}

vec3 TrackBall::getRotationAxis()
{
	return normalize(cross(startingPoint, endingPoint));
}

float TrackBall::getRotationAngle()
{
	return (acos(dot(startingPoint, endingPoint)));
}

vec3 TrackBall::getTranslation()
{
	return normalize(endingPoint - startingPoint)*defaultTranslationStep;
}

void TrackBall::resetPoints()
{
	startingPoint = vec3(0.0);
	endingPoint = vec3(0.0);
}

void TrackBall::setViewport(int chosenWidth, int chosenHeight)
{
	width = chosenWidth;
	height = chosenHeight;
}

vec3 TrackBall::projectToSphere(int x, int y)
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