#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <mmsystem.h>
#include <glut.h>
#include <iostream>
#include <time.h>

#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

int WIDTH = 1280;
int HEIGHT = 720;

GLuint texC;
GLuint texG;
GLuint texS;

char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

class Vector
{
public:
	GLdouble x, y, z;
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 10.0f, float eyeY = 3.0f, float eyeZ = 20.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
};

Camera camera;

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

int cameraZoom = 0;

//Model Variables
Model_3DS model_cat;



// Textures
GLTexture tex_ground;
GLTexture tex_sky;
GLTexture tex_hell_wall;
GLTexture tex_fire;


//cat movement

//absolute
float catx_hell = -17.5;
float caty_hell = 0;
float catz_hell= -14;

//for garden
float catx = -17.5;
float caty = 0;
float catz = -14;

//relative
float catx_add_hell = 0;
float caty_add_hell = 0;
float catz_add_hell = 0;

//for garden
float catx_add = 0;
float caty_add= 0;
float catz_add = 0;

//hellwall
float wallx = 0;

//<<<<<<< Updated upstream
//=======
// garden scene variables
bool clearGardenScene = true; // if true means that reached the house and clear lvl 1
bool lvl_1 = false;
int score = 0;      // for the garden scene score calculations
int hellScore = 0; //to be added to garden score if level cleared
bool hellLost = false;

// coins taken ?
bool goldCoinTaken = false;
bool silverCoinTaken = false;
bool bronzeCoinTaken = false;
//>>>>>>> Stashed changes


//coins taken? hell scene!
bool gold1 = false;
bool gold2 = false;
bool silver1 = false;
bool silver2 = false;
bool bronze1 = false;
bool bronze2 = false;


void sound(int reason) {
	switch (reason) {
	case 0:  //cat moves
		PlaySound("sound_move.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;

	/*case 1: //cat collides
		PlaySound("sound_collide.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;*/

	case 2: //collect coin
		PlaySound("sound-coin.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;

	case 3: //collect coin
		PlaySound("sound_lost.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;
/*	
	case 5: //background
		mciSendString(L"open \"sound_bg.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
		mciSendString(L"play mp3 repeat", NULL, 0, NULL);
		break;*/
	}
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 1000000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void Keyboard(unsigned char key, int x, int y) {
	float d = 0.1;
	float a = 1.0;

	switch (key) {
	case 'w':
		camera.moveY(d);
		break;
	case 's':
		camera.moveY(-d);
		break;
	case 'a':
		camera.moveX(d);
		break;
	case 'd':
		camera.moveX(-d);
		break;
	case 'q':
		camera.moveZ(d);
		break;
	case 'e':
		camera.moveZ(-d);
		break;
	case 'u':
		camera.rotateX(a);
		break;
	case 'j':
		camera.rotateX(-a);
		break;
	case 'h':
		camera.rotateY(a);
		break;
	case 'k':
		camera.rotateY(-a);
		break;

	case GLUT_KEY_ESCAPE:
		exit(EXIT_SUCCESS);
	}
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	

	if (key == GLUT_KEY_UP) {

		
		if (clearGardenScene) {
			float temp = catz_add_hell - 0.5;
			if (temp + catz_hell > -16) {
				catz_add_hell -= 0.5;
				//sound(0);
			}
			else {
				//sound(1);
			}


			if ((catx_add_hell + catx_hell) > 0 && (catx_add_hell + catx_hell) < 3 && (catz_add_hell + catz_hell) > 0 && (catz_add_hell + catz_hell) < 5 && !gold1) {
				// Gold coin
				hellScore += 3;
				gold1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 10 && (catx_add_hell + catx_hell) < 13 && (catz_add_hell + catz_hell) > 16 && (catz_add_hell + catz_hell) < 19 && !gold2) {
				// Gold coin
				//goldCoin(10, 3, 16);
				hellScore += 3;
				gold2 = true;
				sound(2);
			}


			if ((catx_add_hell + catx_hell) > -9 && (catx_add_hell + catx_hell) < -6 && (catz_add_hell + catz_hell) > 8 && (catz_add_hell + catz_hell) < 11 && !silver1) {
				//silverCoin(-9, 3, 8);
				hellScore += 2;
				silver1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > -8 && (catx_add_hell + catx_hell) < -5 && (catz_add_hell + catz_hell) > -16 && (catz_add_hell + catz_hell) < -13 && !silver2) {
				//silverCoin(-8, 3, -16);
				hellScore += 2;
				silver2 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 1 && (catx_add_hell + catx_hell) < 4 && (catz_add_hell + catz_hell) > 5 && (catz_add_hell + catz_hell) < 8 && !bronze1) {
				//bronzeCoin(1, 3, 5);
				hellScore += 1;
				bronze1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 6 && (catx_add_hell + catx_hell) < 9 && (catz_add_hell + catz_hell) > -2 && (catz_add_hell + catz_hell) < 1 && !bronze2) {
				//bronzeCoin(6, 3, -2);
				hellScore += 1;
				bronze2 = true;
				sound(2);
			}

			std::cout << (hellScore) << "score\n";

		}
	}


	else if (key == GLUT_KEY_DOWN) {
			


		/*
			if (lvl_1) {
				// if reached the house then set the flag to true to clear lvl 1
				if (0 < temp1 + catz < 1) {
					if (0 < temp1 + catx < 1) {
						clearGardenScene = true;
						std::cout << (clearGardenScene) << "reached my goal:\n";
						lvl_1 = false;
					}
				}
			}*/

			if (clearGardenScene) {
				float temp1 = catz_add_hell + 0.5;
				if (temp1 + catz_hell < 17.5) {
					catz_add_hell += 0.5;
					//sound(0);
				}
				else {
					//sound(1);
				}

				if ((catx_add_hell + catx_hell) > 0 && (catx_add_hell + catx_hell) < 3 && (catz_add_hell + catz_hell) > 0 && (catz_add_hell + catz_hell) < 5 && !gold1) {
					// Gold coin
					hellScore += 3;
					gold1 = true;
					sound(2);
				}

				if ((catx_add_hell + catx_hell) > 10 && (catx_add_hell + catx_hell) < 13 && (catz_add_hell + catz_hell) > 16 && (catz_add_hell + catz_hell) < 19 && !gold2) {
					// Gold coin
					//goldCoin(10, 3, 16);
					hellScore += 3;
					gold2 = true;
					sound(2);
				}


				if ((catx_add_hell + catx_hell) > -9 && (catx_add_hell + catx_hell) < -6 && (catz_add_hell + catz_hell) > 8 && (catz_add_hell + catz_hell) < 11 && !silver1) {
					//silverCoin(-9, 3, 8);
					hellScore += 2;
					silver1 = true;
					sound(2);
				}

				if ((catx_add_hell + catx_hell) > -8 && (catx_add_hell + catx_hell) < -5 && (catz_add_hell + catz_hell) > -16 && (catz_add_hell + catz_hell) < -13 && !silver2) {
					//silverCoin(-8, 3, -16);
					hellScore += 2;
					silver2 = true;
					sound(2);
				}

				if ((catx_add_hell + catx_hell) > 1 && (catx_add_hell + catx_hell) < 4 && (catz_add_hell + catz_hell) > 5 && (catz_add_hell + catz_hell) < 8 && !bronze1) {
					//bronzeCoin(1, 3, 5);
					hellScore += 1;
					bronze1 = true;
					sound(2);
				}

				if ((catx_add_hell + catx_hell) > 6 && (catx_add_hell + catx_hell) < 9 && (catz_add_hell + catz_hell) > -2 && (catz_add_hell + catz_hell) < 1 && !bronze2) {
					//bronzeCoin(6, 3, -2);
					hellScore += 1;
					bronze2 = true;
					sound(2);
				}


				std::cout << (hellScore) << "score\n";

			}


		}

	
	else if (key == GLUT_KEY_LEFT) {
		

		if (clearGardenScene) {

			float temp2 = catx_add_hell - 0.5;
			if (temp2 + catx_hell > -18) {
				catx_add_hell -= 0.5;
				//sound(0);
			}
			else {
				//sound(1);
			}
			std::cout << (hellLost) << "\n";

			if (((catx_add_hell + catx_hell) < ((wallx - 20)) || ((catx_add_hell + catx_hell) > (-(wallx)+20)))) {
				//so if the cat hits the wall
				hellLost = true;
				sound(3);
			}

			if ((catx_add_hell + catx_hell) > 0 && (catx_add_hell + catx_hell) < 3 && (catz_add_hell + catz_hell) > 0 && (catz_add_hell + catz_hell) < 5 && !gold1) {
				// Gold coin
				hellScore += 3;
				gold1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 10 && (catx_add_hell + catx_hell) < 13 && (catz_add_hell + catz_hell) > 16 && (catz_add_hell + catz_hell) < 19 && !gold2) {
				// Gold coin
				//goldCoin(10, 3, 16);
				hellScore += 3;
				gold2 = true;
				sound(2);
			}


			if ((catx_add_hell + catx_hell) > -9 && (catx_add_hell + catx_hell) < -6 && (catz_add_hell + catz_hell) > 8 && (catz_add_hell + catz_hell) < 11 && !silver1) {
				//silverCoin(-9, 3, 8);
				hellScore += 2;
				silver1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > -8 && (catx_add_hell + catx_hell) < -5 && (catz_add_hell + catz_hell) > -16 && (catz_add_hell + catz_hell) < -13 && !silver2) {
				//silverCoin(-8, 3, -16);
				hellScore += 2;
				silver2 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 1 && (catx_add_hell + catx_hell) < 4 && (catz_add_hell + catz_hell) > 5 && (catz_add_hell + catz_hell) < 8 && !bronze1) {
				//bronzeCoin(1, 3, 5);
				hellScore += 1;
				bronze1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 6 && (catx_add_hell + catx_hell) < 9 && (catz_add_hell + catz_hell) > -2 && (catz_add_hell + catz_hell) < 1 && !bronze2) {
				//bronzeCoin(6, 3, -2);
				hellScore += 1;
				bronze2 = true;
				sound(2);
			}


			std::cout << (hellScore) << "score\n";

		}
	}


	else if (key == GLUT_KEY_RIGHT) {
		
		if (clearGardenScene) {

			float temp3 = catx_add_hell + 0.5;
			if (temp3 + catx_hell < 18) {
				catx_add_hell += 0.5;
				//sound(0);
			}
			else {
				//sound(1);
			}

			std::cout << (hellLost) << "\n";

			if (((catx_add_hell + catx_hell) < ((wallx - 20)) || ((catx_add_hell + catx_hell) > (-(wallx)+20)))) {
				//so if the cat hits the wall
				hellLost = true;
				sound(3);
			}

			if ((catx_add_hell + catx_hell) > 0 && (catx_add_hell + catx_hell) < 3 && (catz_add_hell + catz_hell) > 0 && (catz_add_hell + catz_hell) < 5 && !gold1) {
				// Gold coin
				hellScore += 3;
				gold1 = true;
				sound(2);
			}

			g
			if ((catx_add_hell + catx_hell) > 10 && (catx_add_hell + catx_hell) < 13 && (catz_add_hell + catz_hell) > 16 && (catz_add_hell + catz_hell) < 19 && !gold2) {
				// Gold coin
				//goldCoin(10, 3, 16);
				hellScore += 3;
				gold2 = true;
				sound(2);
			}


			if ((catx_add_hell + catx_hell) > -9 && (catx_add_hell + catx_hell) < -6 && (catz_add_hell + catz_hell) > 8 && (catz_add_hell + catz_hell) < 11 && !silver1) {
				//silverCoin(-9, 3, 8);
				hellScore += 2;
				silver1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > -8 && (catx_add_hell + catx_hell) < -5 && (catz_add_hell + catz_hell) > -16 && (catz_add_hell + catz_hell) < -13 && !silver2) {
				//silverCoin(-8, 3, -16);
				hellScore += 2;
				silver2 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 1 && (catx_add_hell + catx_hell) < 4 && (catz_add_hell + catz_hell) > 5 && (catz_add_hell + catz_hell) < 8 && !bronze1) {
				//bronzeCoin(1, 3, 5);
				hellScore += 1;
				bronze1 = true;
				sound(2);
			}

			if ((catx_add_hell + catx_hell) > 6 && (catx_add_hell + catx_hell) < 9 && (catz_add_hell + catz_hell) > -2 && (catz_add_hell + catz_hell) < 1 && !bronze2) {
				//bronzeCoin(6, 3, -2);
				hellScore += 1;
				bronze2 = true;
				sound(2);
			}


			std::cout << (hellScore) << "score\n";

		}
	}

	glutPostRedisplay();
}
//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(2, 2);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void goldCoin(float x,float y,float z) {
	glPushMatrix();
	// Gold coin
	GLUquadricObj* qobjz;
	qobjz = gluNewQuadric();
	glTranslatef(x, y, z);
	glScaled(2, 2, 2);
	glBindTexture(GL_TEXTURE_2D, texG);
	gluQuadricTexture(qobjz, true);
	gluQuadricNormals(qobjz, GL_SMOOTH);
	gluDisk(qobjz, 0, 0.2, 20, 20);
	gluDeleteQuadric(qobjz);
	glPopMatrix();
}

void silverCoin(float x, float y, float z) {
	glPushMatrix();
	// Silver coin
	GLUquadricObj* qobjy;
	qobjy = gluNewQuadric();
	glTranslatef(x, y, z);
	glScaled(2, 2, 2);
	glBindTexture(GL_TEXTURE_2D, texS);
	gluQuadricTexture(qobjy, true);
	gluQuadricNormals(qobjy, GL_SMOOTH);
	gluDisk(qobjy, 0, 0.2, 20, 20);
	gluDeleteQuadric(qobjy);
	glPopMatrix();
}

void bronzeCoin(float x, float y, float z) {
	glPushMatrix();
	// Bronze coin
	GLUquadricObj* qobjx;
	qobjx = gluNewQuadric();
	glTranslatef(x, y, z);
	glScaled(2, 2, 2);
	glBindTexture(GL_TEXTURE_2D, texC);
	gluQuadricTexture(qobjx, true);
	gluQuadricNormals(qobjx, GL_SMOOTH);
	gluDisk(qobjx, 0, 0.2, 20, 20);
	gluDeleteQuadric(qobjx);
	glPopMatrix();
}

void drawCat(float x, float y, float z) {
	glPushMatrix();

	if (clearGardenScene) {
		glTranslatef(catx_add_hell + catx_hell, caty_add_hell + caty_hell, catz_add_hell + catz_hell);
		glRotatef(180.f, 0, 1, 0);

		//=======
		glScalef(8, 8, 8);
		//>>>>>>> Stashed changes
		model_cat.Draw();
		glPopMatrix();
	}
	

	//std::cout << (catx+catx_add) << "x:\n";

	//std::cout << (caty + caty_add) << "y:\n";

	//std::cout << (catz + catz_add) << "z:\n";
}

void drawHellWall(float x, float y, float z) {
	glColor3f(0, 0, 0);
	glPushMatrix();
	glTranslated(x,y,z);
	glScaled(40, 7.5, 1);
	glutSolidCube(1);
	glPopMatrix();
}

void drawHellWallTex(float x, float y, float z) {
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_hell_wall.texture[0]);

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0+x, 0+y, z);
	glTexCoord2f(1, 0);
	glVertex3f(2.5+x, 0+y, 0+z);
	glTexCoord2f(1, 1);
	glVertex3f(2.5+x, 7.5+y, 0+z);
	glTexCoord2f(0, 1);
	glVertex3f(0+x, 7.5+y, 0+z);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void drawFire(float x, float y, float z) {
	glDisable(GL_LIGHTING);	// Disable lighting 

	//glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_fire.texture[0]);

	glPushMatrix();
	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);	// Set normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(0 + x, 0 + y, z);
	glTexCoord2f(0, 1);
	glVertex3f(1 + x, 0 + y, 0 + z);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(0.5 + x, 6.5 + y, 0 + z);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	//glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

	setupCamera();

	// Draw Ground
	RenderGround();


	//draw cat
	drawCat(catx_add, caty_add, catz_add);

	//draw fire
	drawFire(0, 0, 0);
	

	//back wall
	drawHellWall(0, 3.75, -20);

	for (int i = 0; i < 16;i++) {
		drawHellWallTex(-20+(i*2.5), 0, 0.6 - 20);
	}

	//left wall
	glPushMatrix();
	glRotated(90, 0, 1, 0);
	drawHellWall(0, 3.75, -20 + wallx);

	for (int i = 0; i < 16;i++) {
		drawHellWallTex(-20 + (i * 2.5), 0, 0.6 - 20+wallx);
	}
	glPopMatrix();

	//right wall
	glPushMatrix();
	glRotated(-90, 0, 1, 0);
	glTranslated(0, 0, 0);
	drawHellWall(0, 3.75, -20+wallx);

	for (int i = 0; i < 16;i++) {
		drawHellWallTex(-20 + (i * 2.5), 0, 0.6 - 20 +wallx);
	}
	glPopMatrix();

////////////////////////////////hell scene coins

//<<<<<<< Updated upstream
	// Gold coin
	
//=======
	if (!gold1) {
		// Gold coin
		goldCoin(0, 3, 0);
	}
	
	if (!gold2) {
		// Gold coin
		goldCoin(10, 3, 16);
	}


	if (!silver1) {
		silverCoin(-9, 3, 8);
	}

	if (!silver2) {
		silverCoin(-8, 3, -16);
	}

	if (!bronze1) {
		bronzeCoin(1,3,5);
	}

	if (!bronze2) {
		bronzeCoin(6,3,-2);
	}


	///////////////////////////////////////////
//>>>>>>> Stashed changes

	//sky box
	glPushMatrix();
	//draw the panorama
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(70, 0, 0);
	glRotated(240, 1, 0, 1);
	glColor3f(0.831, 0.925, 0.988);
	glBindTexture(GL_TEXTURE_2D, tex_sky.texture[0]);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 200, 200);
	gluDeleteQuadric(qobj);
	glPopMatrix();

	glutSwapBuffers();
}

//=======================================================================
// Motion Function
//=======================================================================
void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_cat.Load("Models/cat/CatMac.3ds");


	// Loading texture files
	tex_ground.Load("textures/hell-ground.bmp"); // ground
	tex_sky.Load("textures/hell-ground.bmp"); // hell scene
	tex_hell_wall.Load("textures/hell-wall.bmp");
	tex_fire.Load("textures/fire.bmp");
	
	// coins textures
	loadBMP(&texC, "textures/bronze.bmp", false); // texture for bronze coin
	loadBMP(&texG, "textures/gold.bmp", false); // texture for gold coin
	loadBMP(&texS, "textures/silver.bmp", false); // texture for silver coin
	loadBMP(&texS, "textures/silver.bmp", false); // texture for silver coin

}


void wall_collapse(int useless) {
	wallx += (0.1);
	glutTimerFunc(2000, wall_collapse, 0);
	glutPostRedisplay();
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(Keyboard);

	glutSpecialFunc(Special);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

	myInit();

	if (clearGardenScene) {
		glutTimerFunc(9000, wall_collapse, 0);
	}

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}