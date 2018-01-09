#include "stdafx.h"
#include "TrackBall.h"

float TrackBall::translationFactor = 5.0;
float TrackBall::scalingFactor = 5.0;

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
	return normalize(endingPoint - startingPoint)*translationFactor;
}

vec3 TrackBall::getNonUniformScaling()
{
	float dx = endingPoint[0] - startingPoint[0];
	float dy = endingPoint[1] - startingPoint[1];
	//scaling along the x axis
	if (dx > dy)
	{
		//scaling up
		if (dx > 0)
		{
			return vec3(dx, 0, 0)*scalingFactor;
		}
		//scaling down
		else if (dx!=0)
		{
			return vec3(1/dx, 0, 0)/scalingFactor;
		}
		
	}
	//scaling along the y axis
	else
	{
		//scaling up
		if (dy > 0)
		{
			return vec3(0, dy, 0)*scalingFactor;
		}
		//scaling down
		else if (dy != 0)
		{
			return vec3(0, 1/dy, 0)/scalingFactor;
		}
	}
}

vec3 TrackBall::getUniformScaling()
{
	float dy = endingPoint[1] - startingPoint[1];
	float scale = length(endingPoint - startingPoint);
	if (scale == 0)
	{
		return vec3(1, 1, 1);
	}
	//scaling up
	if (dy > 0)
	{
		return vec3(scale)*scalingFactor;
	}
	//scaling down
	else
	{
		return vec3(1/scale)/scalingFactor;
	}
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