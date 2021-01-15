#pragma once
#include "Vector3f.h"

class Camera
{
public:

	Vector3f eye, center, up;

	Camera(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ , float upX , float upY , float upZ );


	void moveX(float d);

	void moveY(float d);

	void moveZ(float d);

	void rotateX(float a);

	void rotateY(float a);

	void look();
};

