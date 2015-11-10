// LandBound.cpp : Gunbound game using c++ and openGL *omega sigh*.

#include "stdafx.h"

#include <string>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <sstream>
#include <math.h>
#include <random>
#include <gl\GL.h>
#include <gl\GLU.h>
#include "GL/freeglut.h"
#pragma comment(lib, "OpenGL32.lib") //specify linker options

using namespace std;

typedef enum { analytical, numerical} integrationMode;
integrationMode integrateMode = numerical;

random_device ran;
mt19937 dev(ran());
mt19937 gen(ran());

//key define
#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_F 0x46
#define VK_L 0x4C
#define VK_SPACE 0x20

//window size and refresh rate
int width = 1000;
int height = 500;
int refresh = 1000 / 60;
//classes
class vec2
{
public:
	float x, y;
	vec2 vecSum(vec2 a, vec2 b)
	{
		vec2 sum;
		sum.x = a.x + b.x;
		sum.y = a.y + b.y;
		return sum;
	}
	int dot(vec2 a, vec2 b)
	{
		int dprod = (a.x*b.x) + (a.y*b.y);
		return dprod;
	}
};

class player
{
	float gaugeX, gaugeY, gaugeH, gaugeW;
	float maxH = gaugeH + 30;
	bool fire = false;
};

// vectors
vec2 bullet1, bullet2, tank1, tank2, velocity1, velocity2, accel1, accel2;

//land specs
float land_width = 100;
float land_height = 50;

//tank specs
float tank_width = 20;
float tank_height = 10;

//powergauge
float gauge_posy = land_height * 3;

float gauge1_height = 1;
float gauge2_height = 1;

float gauge_width = 10;
float gauge_maxheight = gauge_posy + 30;
float gauge_fill = 2;

//scoring
int p1life = 5;
int p2life = 5;

//bullet
float theta1;
float theta2;
float bulletSize = 5; //radius
int bullet_segments = 8;
float windVelocity = 0;
float storewind1 = 0;
float storewind2 = 0;
float ts = 0;

//bools
bool player2 = false;
bool gameStart = false;
bool p1fire = false;
bool p2fire = false;

static float rotAngle1 = 0.0;
static float rotAngle2 = 360.0;
static float g = -9.8;
static float a = 5;

float startTime;
const int milli = 1000;

//function functions

int steprand()
{
	int wind = 5;
	uniform_int_distribution<> dis(-wind, wind);
	int a = dis(dev);
	return a;
}

//int to string function
string inttostr(int x) {
	//converts an int to a string using sstream lib
	stringstream convs;
	convs << x;
	return convs.str();
}

//void functions

//keyboard controls
void keyboard() {
	if (player2 == false && p1fire == false)
	{
		if (GetAsyncKeyState(VK_W))
		{
			if (gauge1_height <= gauge_maxheight)
			{
				gauge1_height += gauge_fill;
			}
		}

		if (GetAsyncKeyState(VK_S))
		{
			if (gauge1_height > 1)
			{
				gauge1_height -= gauge_fill;
			}
		}

		if (GetAsyncKeyState(VK_A))
		{
			if (rotAngle1 <= 180.0)
			{
				rotAngle1 += 1.0;
			}
		}

		if (GetAsyncKeyState(VK_D))
		{
			if (rotAngle1 >= 2)
			{
				rotAngle1 -= 1.0;
			}
		}
		if (GetAsyncKeyState(VK_F))
		{
			accel1.x = gauge1_height / 5;
			accel1.y = gauge1_height / 5;
			p1fire = true;
			
			
			bullet2.x = tank2.x + tank_width / 2;
			bullet2.y = tank2.y + (tank_height + 5);
			velocity2.x = 0;
			velocity2.y = 0;
			storewind2 = 0;
			storewind1 = windVelocity;
			bullet1.x = tank1.x + (tank_width / 2);
			bullet1.y = tank1.y + (tank_height + 5);
			theta1 = (3.1415926 / 180) * rotAngle1;
			
		}
	}
	if (player2 == true && p2fire == false)
	{
		if (GetAsyncKeyState(VK_UP))
		{
			if (gauge2_height <= gauge_maxheight)
			{
				gauge2_height += gauge_fill;
			}
		}

		if (GetAsyncKeyState(VK_DOWN))
		{
			if (gauge2_height >= 1)
			{
				gauge2_height -= gauge_fill;
			}
		}

		if (GetAsyncKeyState(VK_RIGHT))
		{
			if (rotAngle2 >= 180.0)
			{
				rotAngle2 -= 1.0;
			}
		}

		if (GetAsyncKeyState(VK_LEFT))
		{
			if (rotAngle2 <= 360.0)
			{
				rotAngle2 += 1.0;
			}
		}
		if (GetAsyncKeyState(VK_L))
		{
			accel2.x = gauge2_height / 5 + windVelocity;
			accel2.y = gauge2_height / 5 + g;
			p2fire = true;
		
			bullet1.x = tank1.x + tank_width / 2;
			bullet1.y = tank1.y + (tank_height + 5);
			velocity1.x = 0;
			velocity1.y = 0;
			storewind1 = 0;
			storewind2 = windVelocity;
			bullet2.x = tank2.x + (tank_width / 2);
			bullet2.y = tank2.y + (tank_height / 2);
			theta2 = (3.1415926 / 180) * (rotAngle2 - 180);
			
		}
	}
}

//makes gl recognized 2d usage
void use2D(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, width, 0.0f, height, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//prints text on screen
void textDraw(float x, float y, string text) {
	glRasterPos2f(x, y);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char*)text.c_str());
}

//draw paddles
void boxDraw(float x, float y, float width, float height) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
}

//line draw
void lineDraw(float x1, float y1, float x2, float y2) {
	glPushMatrix();
	glTranslatef(x1, y1, 0.0f);
	glRotatef(rotAngle1, 0.0f, 0.0f, 1.0f);
	glTranslatef(-x1, -y1, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0.0f);
	glVertex3f(x2, y2, 0.0f);
	glEnd();
	glPopMatrix();
}

void line2Draw(float x1, float y1, float x2, float y2) {
	glPushMatrix();
	glTranslatef(x1, y1, 0.0f);
	glRotatef(rotAngle2, 0.0f, 0.0f, 1.0f);
	glTranslatef(-x1, -y1, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, 0.0f);
	glVertex3f(x2, y2, 0.0f);
	glEnd();
	glPopMatrix();
}

//draw the ball
void ballDraw(float cx, float cy, float r, int segments) {
	float theta = 2 * 3.1415926 / float(segments);
	//sin and cos calculation
	float cos = cosf(theta);
	float sin = sinf(theta);
	float t;

	float varx = r; //start at angle 0
	float vary = 0;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < segments; i++) {
		glVertex2f(varx + cx, vary + cy); //outputs vertex

										  //apply rotation matrix
		t = varx;
		varx = cos * varx - sin * vary;
		vary = sin * t + cos * vary;
	}
	glEnd();
}

//event check and cleanup
void boom()
{
	if (player2 == false)
	{
		bullet1.x = tank1.x + (tank_width / 2);
		bullet1.y = tank1.y + (tank_height / 2);
		windVelocity = steprand();
		p1fire = false;
		player2 = true;

	}
	else if (player2 == true)
	{
		bullet2.x = tank2.x + (tank_width / 2);
		bullet2.y = tank2.y + (tank_height / 2);
		windVelocity = steprand();
		p2fire = false; 
		player2 = false;

	}
}

//collision check
void collisionChecker() {
	//left wall collision
	if ((bullet1.x < 0) || (bullet2.x < 0)) {
		boom();
	}

	//right wall collision
	if ((bullet1.x > width) || (bullet2.x > width)) {
		boom();
	}

	//top /bot wall collision
	if ((bullet1.y > height - 20 || bullet1.y < 10) || (bullet2.y > height - 20 || bullet2.y < 10))
	{
		boom();
	}

	//tank collision
	if ((bullet2.x >= tank1.x) &&
		(bullet2.x <= tank1.x + tank_width) &&
		(bullet2.y <= tank1.y + tank_height) &&
		(bullet2.y >= tank1.y))
	{
		if (player2 == true)
		{
			p1life -= 1;
		}
		boom();
	}
	if ((bullet1.x >= tank2.x) &&
		(bullet1.x <= tank2.x + tank_width) &&
		(bullet1.y <= tank2.y + tank_height) &&
		(bullet1.y >= tank2.y))
	{
		if (player2 == false)
		{
			p2life -= 1;
		}
		boom();
	}
}

void updateProjectileStateAnalytical(float t) {

	bullet1.x = velocity1.x * t + cos(rotAngle1);
	bullet1.y = t * (0.5 * g * t + velocity1.y) + sin(rotAngle1);
}

void updateProjectileStateNumerical(float t, float dt) {
	/*
	//euler
	//pos
	bullet1.x += bullet1.x * dt + cos(rotAngle1);
	bullet1.y += bullet1.y * dt - sin(rotAngle1);

	//vel
	velocity1.y += g * dt;
	*/

	float prevtime = t;
	float pprevtime = prevtime;
	float prevdt = dt;

	//time corrected verlet
	if (p1fire == true) {
		float prevposx1 = bullet1.x;
		float prevposy1 = bullet1.y;

		bullet1.x += cos(theta1)*(bullet1.x * dt + ((gauge1_height / 10) + windVelocity) * 0.5) ;
		bullet1.y += sin(theta1)*(bullet1.y * dt + dt * dt * g * 0.5);
		//bullet1.x += (bullet1.x + (bullet1.x - prevposx1)*(dt / prevdt) + (t - pprevtime)*(t - prevtime)*((gauge1_height / 10) + windVelocity) * 0.5);
		//bullet1.y += (bullet1.y + (bullet1.y - prevposy1)*(dt / prevdt) + (t - pprevtime)*(t - prevtime)*(g)* 0.5);
	}
	
	if (p2fire == true) {
		float prevposx2 = bullet2.x;
		float prevposy2 = bullet2.y;

		bullet2.x -= (bullet2.x * dt + ((gauge2_height / 10) + windVelocity) * 0.5);
		bullet2.y -= (bullet2.y * dt + dt * dt * g * 0.5);
		//bullet2.x += bullet2.x + (bullet2.x - prevposx2)*(dt / prevdt) + accel2.x * dt * dt;
		//bullet2.y += bullet2.y + (bullet2.y - prevposy2)*(dt / prevdt) + accel2.y * dt * dt;
	}
}

void updateProjectileState(float t, float dt) {
	if (integrateMode == analytical) {
		updateProjectileStateAnalytical(t);
	}
	else {
		updateProjectileStateNumerical(t, dt);
	}
}

void bulletMove(float t, float dt)
{
	if (player2 == false)
	{
		if (p1fire == true) {
			updateProjectileState(t, dt);
			collisionChecker();
		}
		/*
		velocity1.x += accel1.x*ts;
		velocity1.y += accel1.y*ts;
		bullet1.x += (velocity1.x * ts) + ((accel1.x + storewind1)*(ts*ts)) / 2;
		bullet1.y += (velocity1.y * ts) + ((accel1.y - 9.8)*(ts*ts)) / 2;
		ts += 60 / 1000;
		*/
		
	}
	else if (player2 == true)
	{
		if (p2fire == true) {
			updateProjectileState(t, dt);
			collisionChecker();
		}
		/*
		velocity2.x += accel2.x*ts;
		velocity2.y += accel2.y*ts;
		bullet2.x += (velocity2.x * ts) + ((accel2.x + storewind2)*(ts*ts)) / 2;
		bullet2.y += (velocity2.y * ts) + ((accel2.y - 9.8)*(ts*ts)) / 2;
		ts += 60 / 1000;
		collisionChecker();
		*/
	}
}


//draw on screen
void draw() {
	//clearing the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//put draw codes below

	//ball display
	ballDraw(bullet1.x, bullet1.y, bulletSize, bullet_segments);
	ballDraw(bullet2.x, bullet2.y, bulletSize, bullet_segments);

	//ballDraw(ball2_posx, ball2_posy, ball_radius, ball_segments);

	//land display
	boxDraw(0, 0, land_width, land_height);
	boxDraw(width - land_width - 8, 0, land_width, land_height);

	//tank draw
	boxDraw(tank1.x, tank1.y, tank_width, tank_height);
	boxDraw(tank2.x, tank2.y, tank_width, tank_height);

	//gauge draw
	boxDraw(tank1.x, gauge_posy, gauge_width, gauge1_height);
	boxDraw(tank2.x, gauge_posy, gauge_width, gauge2_height);

	//line
	lineDraw((tank1.x + (tank_width / 2)), (tank1.y + (tank_height / 2)), (tank1.x + 50), (tank1.y + (tank_height / 2)));
	line2Draw((tank2.x + (tank_width / 2)), (tank2.y + (tank_height / 2)), (tank2.x - 30), (tank2.y + (tank_height / 2)));


	//score display
	textDraw(width / 2 - 30, height - 30, inttostr(rotAngle1) + " " + inttostr(p1life) + " : " + inttostr(p2life) + " " + inttostr(abs(rotAngle2 - 360)));
	textDraw(width / 2, height - 50, inttostr(windVelocity));
	//swapping buffers
	glutSwapBuffers();
}

//screen update handler
void update(void) {
	//input
	keyboard();

	//timer?
	static float prevTime = -1;
	float t, dt;

	t = glutGet(GLUT_ELAPSED_TIME) / (float)milli - startTime;

	if (prevTime < 0.0) {
		prevTime = t;
		return;
	}

	dt = t - prevTime;
	bulletMove(t, dt);
	prevTime = t;

	//Redisplay Frame
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	//initiliaze opengl using glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Le GunBound");
	//add values to vectors
	//tank position
	tank1.x = land_width / 2;
	tank1.y = land_height;
	tank2.x = width - (land_width / 2) - 8 - bullet1.x;
	tank2.y = land_height;
	if (gameStart == false) {
		//bullet positions
		bullet1.x = tank1.x + tank_width / 2;
		bullet1.y = tank1.y + (tank_height + 5);
		bullet2.x = tank2.x + tank_width / 2;
		bullet2.y = tank2.y + (tank_height + 5);
	}
	//uses the void functions
	glutDisplayFunc(draw);
	
	//setup the 2d and set draw color to black
	use2D(width, height);
	glColor3f(1.0f, 1.0f, 1.0f);

	//refresh rate
	glutIdleFunc(update);

	//program loop
	glutMainLoop();

	return 0;
}

//wake me up
//wake me up inside
//cant wake up
//wake me up inside
//save meeee
