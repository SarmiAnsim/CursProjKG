#include "Render.h"
#include <sstream>
#include <iostream>


#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"
#include <Math.h>

#include <vector>

using namespace std;
#define Vector std::vector

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;

bool launched = false;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()

int kef = 0; 
Vector3 scatter[16] = {
	{1, 0, 0},
	{-1, 0, 0},
	{0, 1, 0},
	{0, -1, 0},

	{sqrt(3)/2, 1./2, 0},
	{-sqrt(3) / 2, -1. / 2, 0},
	{-sqrt(3) / 2, 1. / 2, 0},
	{sqrt(3) / 2, -1. / 2, 0},

	{sqrt(2)/2, sqrt(2)/2, 0},
	{-sqrt(2) / 2, -sqrt(2) / 2, 0},
	{-sqrt(2) / 2, sqrt(2) / 2, 0},
	{sqrt(2) / 2, -sqrt(2) / 2, 0},

	{1. / 2,sqrt(3) / 2,0},
	{-1. / 2,sqrt(3) / 2,0},
	{-1. / 2,-sqrt(3) / 2,0},
	{1. / 2,-sqrt(3) / 2,0}
};

class Particle {
public:
	Vector3 pos;
	Vector3 direction;
	double maxLifeTime = 2;
	double LifeTime;
	double speed;
	double color;

	Particle(Vector3 position, Vector3 dir, double spd)
	{
		pos = position;
		direction = dir;
		LifeTime = 0;
		speed = spd;
		color = LifeTime / maxLifeTime / 2 + 0.5;
	}

	void Draw();

	void Move()
	{
		if (
			(pos.Z() < 1.39 && pos.Z() > 1.36 &&
			pos.X() < 3.7375 && pos.X() > 1.8625 &&
			pos.Y() < 0.625 && pos.Y() > -0.625) || 
			(pos.Z() < 1.15 && pos.Z() > 1.12 &&
			pos.X() < 4.5 && pos.X() > -2 &&
			pos.Y() < 1.5 && pos.Y() > -1.5)
			)
		{ 
			direction = { direction.X(), direction.Y(), 0 };
			direction.normolize();
			if (direction.X() == 0 && direction.Y() == 0)
			{
				direction = direction + scatter[kef++];
				kef %= 15;
			}
		}
		direction.normolize();
		pos = { pos.X() + direction.X() / 150 * speed, pos.Y() + direction.Y() / 150 * speed, pos.Z() + direction.Z() / 150 * speed };
		speed -= speed / 100;
		LifeTime += 0.01;
		color = LifeTime / maxLifeTime / 2 + 0.5;
		Draw();
	}
};

class AllParticles
{
public:
	Vector<Particle> particles;

	void Add(Vector3 position, Vector3 dir, double spd, double rockSpeed)
	{
		Particle tmp1(position, dir, spd);
		particles.push_back(tmp1);
	}

	void Draw()
	{
		glPushMatrix();
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		for (int i = 0; i < particles.size(); i++)
		{
			if (particles[i].maxLifeTime > particles[i].LifeTime)
				particles[i].Move();
			else
				particles.erase(particles.begin() + i--);
		}
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		glPopMatrix();
	}
}Particles;

int numbP = 0;

Texture FlameTex;

class Flame
{
public:
	Vector3 pos;
	Vector3 direction;
	Vector3 vertex[4][4];
	Vector<Vector3> VertexsStart;
	Vector<Vector3> Vertexs;

	Flame(double R, double accur)
	{
		direction = { 0, 0, -0.5 };
		pos = { 2.8, -1, 3.5 };

		vertex[0][0] = Vector3(R * cos(0) * 0.9, R * sin(0) * 0.9, 0);
		vertex[0][1] = Vector3(R * cos(M_PI / 6), R * sin(M_PI / 6), 0);
		vertex[0][2] = Vector3(R * cos(M_PI / 3), R * sin(M_PI / 3), 0);
		vertex[0][3] = Vector3(R * cos(M_PI / 2) * 0.9, R * sin(M_PI / 2) * 0.9, 0);

		vertex[1][0] = Vector3(R * cos(-M_PI / 6), R * sin(-M_PI / 6), 0);
		vertex[1][1] = Vector3(R / 3, 0, direction.Z());
		vertex[1][2] = Vector3(0, R / 3, direction.Z());
		vertex[1][3] = Vector3(R * cos(2 * M_PI / 3), R * sin(2 * M_PI / 3), 0);

		vertex[2][0] = Vector3(R * cos(-M_PI / 3), R * sin(-M_PI / 3), 0);
		vertex[2][1] = Vector3(0, -R / 3, direction.Z());
		vertex[2][2] = Vector3(-R / 3, 0, direction.Z());
		vertex[2][3] = Vector3(R * cos(5 * M_PI / 6), R * sin(5 * M_PI / 6), 0);

		vertex[3][0] = Vector3(R * cos(3 * M_PI / 2) * 0.9, R * sin(3 * M_PI / 2) * 0.9, 0);
		vertex[3][1] = Vector3(R * cos(-2 * M_PI / 3), R * sin(-2 * M_PI / 3), 0);
		vertex[3][2] = Vector3(R * cos(-5 * M_PI / 6), R * sin(-5 * M_PI / 6), 0);
		vertex[3][3] = Vector3(R * cos(M_PI) * 0.9, R * sin(M_PI) * 0.9, 0);

		for (double u = 0; u <= 0.9001; u += accur)
		{
			for (double v = 0; v <= 0.9001; v += accur)
			{
				Vector3 P1 = f3(4, 4, u, v);
				Vector3 P2 = f3(4, 4, u + accur, v);
				Vector3 P3 = f3(4, 4, u, v + accur);
				Vector3 P4 = f3(4, 4, u + accur, v + accur);

				VertexsStart.push_back(P1);
				VertexsStart.push_back(P2);
				VertexsStart.push_back(P3);
				VertexsStart.push_back(P4);
			}
		}

	}

	void Draw(Vector3 pos, Vector3 dir, double speed, Vector3 direction, int change)
	{
		double u = 0, v = 0;

		FlameTex.bindTexture();
		glDisable(GL_LIGHTING);
		glColor3d(1, 1, 1);
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < Vertexs.size() - 3; i += 4)
		{
			double lpoint1 = (Vertexs[i]-pos).length();
			if (lpoint1 <= 0.05)
				lpoint1 = 0;
			double lpoint2 = (Vertexs[i+1] - pos).length();
			if (lpoint2 <= 0.05)
				lpoint2 = 0;
			double lpoint3 = (Vertexs[i+2] - pos).length();
			if (lpoint3 <= 0.05)
				lpoint3 = 0;
			double lpoint4 = (Vertexs[i+3] - pos).length();
			if (lpoint4 <= 0.05)
				lpoint4 = 0;

			glTexCoord2d(u, v);
			glVertex3dv((Vertexs[i] - direction * lpoint1 * (((double)change/10)*0.8 + speed * 0.01)).toArray());
			glTexCoord2d(u + 0.2, v);
			glVertex3dv((Vertexs[i + 1] - direction * lpoint2 * (((double)change / 10)*0.8 + speed * 0.01)).toArray());
			glTexCoord2d(u, v + 0.2);
			glVertex3dv((Vertexs[i + 2] - direction * lpoint3 * (((double)change / 10)*0.8 + speed * 0.01)).toArray());

			glTexCoord2d(u + 0.2, v);
			glVertex3dv((Vertexs[i + 1] - direction * lpoint2 * (((double)change / 10)*0.8 + speed * 0.01)).toArray());
			glTexCoord2d(u, v + 0.2);
			glVertex3dv((Vertexs[i + 2] - direction * lpoint3 * (((double)change / 10)*0.8 + speed * 0.01)).toArray());
			glTexCoord2d(u + 0.2, v + 0.2);
			glVertex3dv((Vertexs[i + 3] - direction * lpoint4 * (((double)change / 10)*0.8 + speed * 0.01)).toArray());

			if(numbP%4 == 0)
				Particles.Add(Vertexs[i+numbP/4], dir, 10, speed);
			numbP = ++numbP % 16;
			if (v + 0.2 == 1)
			{ 
				u += 0.2;
				v = 0;
			}
			else
				v += 0.2;
		}
		glEnd();

		if (lightMode)
			glEnable(GL_LIGHTING);
	}

	void Move(Vector3 pos, Vector3 dir, double speed, double Turn, double AN, double TurnODir, Vector3 normal, Vector3 direction, double change)
	{
		for (Vector3 tmp : VertexsStart)
			Vertexs.push_back(tmp);

		for (int i = 0; i < Vertexs.size(); i++)
		{
			Vertexs[i] = rootN(Turn, { 0,0,1 }, Vertexs[i]);
			Vertexs[i] = rootN(AN, normal, Vertexs[i]);
			Vertexs[i] = rootN(TurnODir, direction, Vertexs[i]);
			Vertexs[i] = Vertexs[i] + pos;
		}

		Draw(pos, dir, speed, direction, change);

		Vertexs.clear();
	}

	Vector3 rootN(double angle, Vector3 norm, Vector3 dir)
	{
		double M[][3] =
		{
		  {{cos(angle) + (1 - cos(angle)) * norm.X() * norm.X()},
		  {(1 - cos(angle)) * norm.X() * norm.Y() - sin(angle) * norm.Z()},
		  {(1 - cos(angle)) * norm.X() * norm.Z() + sin(angle) * norm.Y()}},

		  {{(1 - cos(angle)) * norm.Y() * norm.X() + sin(angle) * norm.Z()},
		  {cos(angle) + (1 - cos(angle)) * norm.Y() * norm.Y()},
		  {(1 - cos(angle)) * norm.Y() * norm.Z() - sin(angle) * norm.X()}},

		  {{(1 - cos(angle)) * norm.Z() * norm.X() - sin(angle) * norm.Y()},
		  {(1 - cos(angle)) * norm.Z() * norm.Y() + sin(angle) * norm.X()},
		  {cos(angle) + (1 - cos(angle)) * norm.Z() * norm.Z()}}
		};

		Vector3 rez = {
			dir.X() * M[0][0] + dir.Y() * M[0][1] + dir.Z() * M[0][2],
			dir.X() * M[1][0] + dir.Y() * M[1][1] + dir.Z() * M[1][2],
			dir.X() * M[2][0] + dir.Y() * M[2][1] + dir.Z() * M[2][2] };
		return rez;
	}

	long double fact(int N)
	{
		if (N < 0)
			return 0;
		if (N == 0)
			return 1;
		else
			return N * fact(N - 1);
	}

	double polyn_Bernstein(int N, int i, double t)
	{
		return fact(N) / (fact(i) * fact(N - i)) * pow(t, i) * pow(1 - t, N - i);
	}

	Vector3 f3(int N, int M, double u, double v)
	{
		Vector3 S = { 0, 0, 0 };
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < M; j++)
			{
				double tmp = polyn_Bernstein(N - 1, i, u) * polyn_Bernstein(M - 1, j, v);
				S = S + Vector3(tmp * vertex[i][j].X(), tmp * vertex[i][j].Y(), tmp * vertex[i][j].Z());
			}
		}
		return S;
	}
} flame(0.05, 0.2);

int vibr = 0;
Vector3 bias[8] = {
	{0,0,0},
	{0,0,1},
	{0,1,0},
	{0,1,1},
	{1,0,0},
	{1,0,1},
	{1,1,0},
	{1,1,1}
};

int changeFlyme = 0;
int changebias = 1;

class AllRocket
{
public:
	ObjFile rocket;
	Texture rocketTex;
	Vector3 pos;
	Vector3 eng_pos[4];
	Vector3 direction;
	Vector3 side;
	double speed;
	double turn;
	double turnOdir;
	

	AllRocket()
	{
		direction = { 0, 0, 1 };
		side = { 1,0,0 };
		pos = { 2.8, 0, 1.9 };
		eng_pos[0] = { 0, 0.125, -0.4 };
		eng_pos[1] = { -0.125, 0, -0.4 };
		eng_pos[2] = { 0, -0.125, -0.4 };
		eng_pos[3] = { 0.125, 0, -0.4 };
		speed = 0;
		turn = 0;
		turnOdir = 0;
	}

	void reset()
	{
		direction = { 0, 0, 1 };
		side = { 1,0,0 };
		pos = { 2.8, 0, -0.6 };
		speed = 0;
		turn = 0;
	}

	double AN()
	{
		double angle = direction.Z();
		angle = acos(angle);

		return angle/M_PI * 180;
	}

	Vector3 norm(Vector3 A, Vector3 B, Vector3 C)
	{
		if (B == C * -1)
			return side;

		double Ak = A.Y() * (B.Z() - C.Z()) + B.Y() * (C.Z() - A.Z()) + C.Y() * (A.Z() - B.Z());
		double Bk = A.Z() * (B.X() - C.X()) + B.Z() * (C.X() - A.X()) + C.Z() * (A.X() - B.X());
		double Ck = A.X() * (B.Y() - C.Y()) + B.X() * (C.Y() - A.Y()) + C.X() * (A.Y() - B.Y());

		double x = Ak;
		double y = Bk;
		double z = Ck;

		double l = sqrt(x * x + y * y + z * z);
		if (l != 0)
		{
			x /= l;
			y /= l;
			z /= l;
		}
		else
			z = 1;

		return { x,y,z };
	}

	void Draw()
	{
		eng_pos[0] = { 0, 0.125, -0.4 };
		eng_pos[1] = { -0.125, 0, -0.4 };
		eng_pos[2] = { 0, -0.125, -0.4 };
		eng_pos[3] = { 0.125, 0, -0.4 };

		Vector3 normal = norm({0,0,0}, {0,0,1}, direction);
		glPushMatrix();
		if(speed >= 1)
		{ 
			glTranslated(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01, 0);

			eng_pos[0] = eng_pos[0] + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01, 0);
			eng_pos[1] = eng_pos[1] + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01, 0);
			eng_pos[2] = eng_pos[2] + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01, 0);
			eng_pos[3] = eng_pos[3] + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01, 0);

		}
		glTranslated(pos.X(), pos.Y(), pos.Z());
		glRotated(turnOdir, direction.X(), direction.Y(), direction.Z());
		glRotated(AN(), normal.X(), normal.Y(), normal.Z());
		glRotated(turn, 0, 0, 1);
		glScaled(0.04, 0.04, 0.04);
		rocketTex.bindTexture();
		rocket.DrawObj();
		glPopMatrix();

		//draw_side();
		if(launched)
		{ 
			for (int i = 0; i < 4; i++)
			{
				eng_pos[i] = rootN(ToRad(turn), { 0,0,1 }, eng_pos[i]);
				eng_pos[i] = rootN(ToRad(AN()), normal, eng_pos[i]);
				eng_pos[i] = rootN(ToRad(turnOdir), direction, eng_pos[i]);

				eng_pos[i] = eng_pos[i] + pos;
			}

			flame.Move(eng_pos[0] + direction * -0.1, direction*-1 + Vector3(bias[vibr].X() * 0.01 * changebias, bias[vibr].Y() * 0.01, 0) * changebias,
				speed, ToRad(turn), ToRad(AN()), ToRad(turnOdir), normal, direction, changeFlyme);
			flame.Move(eng_pos[1] + direction * -0.1, direction*-1 + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01 * changebias, 0) * -changebias,
				speed, ToRad(turn), ToRad(AN()), ToRad(turnOdir), normal, direction, changeFlyme);
			flame.Move(eng_pos[2] + direction * -0.1, direction*-1 + Vector3(bias[vibr].X() * 0.01 * -changebias, bias[vibr].Y() * 0.01, 0) * changebias,
				speed, ToRad(turn), ToRad(AN()), ToRad(turnOdir), normal, direction, changeFlyme);
			flame.Move(eng_pos[3] + direction * -0.1, direction*-1 + Vector3(bias[vibr].X() * 0.01, bias[vibr].Y() * 0.01 * changebias, 0) * -changebias,
				speed, ToRad(turn), ToRad(AN()), ToRad(turnOdir), normal, direction, changeFlyme);

			changeFlyme = ++changeFlyme % 10;
		}

		changebias *= -1;
		vibr = (vibr + 2) % 7;
	}

	void draw_side()
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		double A[3] = { 0 };
		glPushMatrix();
		glTranslated(pos.X(), pos.Y(), pos.Z());
		glScaled(1, 1, 1);
		glColor3d(0, 0, 0);
		glBegin(GL_LINES);
		glVertex3dv(A);
		glVertex3dv(side.toArray());
		glEnd();
		glPopMatrix();

		if (textureMode)
			glEnable(GL_TEXTURE_2D);

		if (lightMode)
			glEnable(GL_LIGHTING);
	}

	void Move(double spd)
	{
		double real_speed = 0;
		if (!speed)
			speed = spd;
		if (speed >= 1)
			real_speed = speed;
		direction.normolize();
		pos = {pos.X() + direction.X()/100 * real_speed, pos.Y() + direction.Y() / 100 * real_speed, pos.Z() + direction.Z() / 100 * real_speed };
		Draw();
	}

	double ToRad(double angl)
	{
		return angl / 180 * M_PI;
	}

	Vector3 rootN(double angle, Vector3 norm, Vector3 dir)
	{
		double M[][3] = 
		{
		  {{cos(angle) + (1 - cos(angle)) * norm.X() * norm.X()},
		  {(1 - cos(angle)) * norm.X() * norm.Y() - sin(angle) * norm.Z()},
		  {(1 - cos(angle)) * norm.X() * norm.Z() + sin(angle) * norm.Y()}},

		  {{(1 - cos(angle)) * norm.Y() * norm.X() + sin(angle) * norm.Z()},
		  {cos(angle) + (1 - cos(angle)) * norm.Y() * norm.Y()},
		  {(1 - cos(angle)) * norm.Y() * norm.Z() - sin(angle) * norm.X()}},

		  {{(1 - cos(angle)) * norm.Z() * norm.X() - sin(angle) * norm.Y()},
		  {(1 - cos(angle)) * norm.Z() * norm.Y() + sin(angle) * norm.X()},
		  {cos(angle) + (1 - cos(angle)) * norm.Z() * norm.Z()}}
		};

		Vector3 rez = {
			dir.X() * M[0][0] + dir.Y() * M[0][1] + dir.Z() * M[0][2],
			dir.X() * M[1][0] + dir.Y() * M[1][1] + dir.Z() * M[1][2],
			dir.X() * M[2][0] + dir.Y() * M[2][1] + dir.Z() * M[2][2] };
		return rez;
	}

	void TurnWS(int ForB)
	{
		direction = rootN(-M_PI / 180 * ForB, side, direction);
		direction.normolize();

		double x = direction.X(), y = direction.Y(), z = direction.Z();
		if (abs(direction.X()) < 0.00000001)
			x = 0;
		if (abs(direction.Y()) < 0.00000001)
			y = 0;
		if (abs(direction.Z()) > 1)
			z = abs(direction.Z()) / direction.Z();
		direction.setCoords(x, y, z);
	}

	void TurnAD(int RorL, bool OzTurnMode)
	{
		if(!OzTurnMode)
		{ 
			direction = rootN(M_PI / 180* RorL, {0,0,1}, direction);
			side = rootN(M_PI / 180 * RorL, { 0,0,1 }, side);
			turn += 1 * (double)RorL;
			if (abs(turn) > 360)
				turn -= 360 * (abs(turn) / turn);
		}
		else
		{
			turnOdir += 2 * (double)RorL;
			if (abs(turnOdir) > 360)
				turnOdir -= 360 * (abs(turnOdir) / turnOdir);
		}
	}

}Rocket;

class AllButton 
{
public:
	ObjFile button;
	Texture buttonTex;
	Vector3 start_pos;
	Vector3 pos;
	int dir;

	AllButton()
	{
		start_pos = { -0.63, 0, 0.90 };
		pos = start_pos;
		dir = 0;
	}

	void Draw()
	{
		glPushMatrix();
		glTranslated(pos.X(), pos.Y(), pos.Z());
		glRotated(90, 0, 0, 1);
		glScaled(0.9, 0.9, 0.9);
		buttonTex.bindTexture();
		button.DrawObj();
		glPopMatrix();
	}

	void Move()
	{
		pos = { pos.X(), pos.Y(), pos.Z() + (0.3 / 20.) * dir };
		Draw();
		if (pos.Z() == start_pos.Z() && dir == 1)
			dir = 0;
	}

	int Where()
	{
		if (pos.Z() >= start_pos.Z())
			return 1;
		if (pos.Z() <= start_pos.Z() - 0.3)
			return -1;
		if(pos.Z() > start_pos.Z() - 0.3)
			return 0;
	}

}Button;


bool replacement = false;

class AllPlatform
{
public:
	Vector3 start_pos;
	Vector3 pos;
	Vector3 points[8] = {
		{-0.9375, -0.625, 0},
		{-0.9375, -0.625, -0.1},

		{0.9375, -0.625, -0.1},
		{0.9375, -0.625, 0},

		{0.9375, 0.625, 0},
		{0.9375, 0.625, -0.1},

		{-0.9375, 0.625, -0.1},
		{-0.9375, 0.625, 0},
	};
	int dir;

	AllPlatform()
	{
		start_pos = { 2.8, 0, 1.375 };
		pos = start_pos;
		dir = 0;
	}

	void Draw()
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslated(pos.X(), pos.Y(), pos.Z());
		glColor3d(0.1, 0.1, 0.1);
		glBegin(GL_QUADS);

		glNormal3d(0, 0, 0.5);
		glVertex3dv(points[0].toArray());
		glVertex3dv(points[3].toArray());
		glVertex3dv(points[4].toArray());
		glVertex3dv(points[7].toArray());

		glNormal3d(0, 0, -0.5);
		glVertex3dv(points[1].toArray());
		glVertex3dv(points[2].toArray());
		glVertex3dv(points[5].toArray());
		glVertex3dv(points[6].toArray());

		glNormal3d(0, -0.5, 0);
		glVertex3dv(points[0].toArray());
		glVertex3dv(points[1].toArray());
		glVertex3dv(points[2].toArray());
		glVertex3dv(points[3].toArray());

		glNormal3d(0.5, 0, 0);
		glVertex3dv(points[2].toArray());
		glVertex3dv(points[3].toArray());
		glVertex3dv(points[4].toArray());
		glVertex3dv(points[5].toArray());

		glNormal3d(0, 0.5, 0);
		glVertex3dv(points[4].toArray());
		glVertex3dv(points[5].toArray());
		glVertex3dv(points[6].toArray());
		glVertex3dv(points[7].toArray());

		glNormal3d(-0.5, 0, 0);
		glVertex3dv(points[6].toArray());
		glVertex3dv(points[7].toArray());
		glVertex3dv(points[0].toArray());
		glVertex3dv(points[1].toArray());

		glEnd();
		glPopMatrix();

		if (textureMode)
			glEnable(GL_TEXTURE_2D);
	}

	void Move()
	{
		if (dir == 1)
			Rocket.pos = { Rocket.pos.X(), Rocket.pos.Y(), Rocket.pos.Z() + (2.5 / 40.) };
		pos = { pos.X(), pos.Y(), pos.Z() + (2.5 / 40.) * dir };
		Draw();
		if (pos.Z() == start_pos.Z() && dir == 1)
		{ 
			dir = 0;
			replacement = false;
		}
	}

	int Where()
	{
		if (pos.Z() >= start_pos.Z())
			return 1;
		if (pos.Z() <= start_pos.Z() - 2.5)
			return -1;
		if (pos.Z() > start_pos.Z() - 2.5)
			return 0;
	}
}Platform;


Shader s[10];  //массивчик для десяти шейдеров

float Time = 0;
int tick_o = 0;
int tick_n = 0;

class Batterfly
{
public:
	Texture BTex, BNM;
	Vector3 pos = { 0,0,0 };
	Vector3 direction = { 1,0,0 };
	Vector<Vector3> track;
	int iter = 1;
	double angle;

	Batterfly(Vector3 points[4])
	{
		pos = { 0,0,0 };
		direction = { 1,0,0 };
		iter = 1;
		for (double t = 0; t <= 1.0001; t += 0.001)
		{
			track.push_back(Beze(points[0], points[1], points[2], points[3], t));
		}
	}

	void Draw(int numb);
	
	void Move(int numb)
	{
		Vector3 dir = (track[iter] - pos).normolize();
		angle = acos(dir.X()) * 180 / M_PI;
		if (dir.Y() < 0)
			angle *= -1;
		pos = track[iter++];

		Draw(numb);
		if (iter >= track.size())
			iter = 0;
	}

	Vector3 Ermit(Vector3 P1, Vector3 P2, Vector3 P3, Vector3 P4, double t)
	{
		return Vector3(
				f1(P1.X(), P2.X(), P3.X(), P4.X(), t),
				f1(P1.Y(), P2.Y(), P3.Y(), P4.Y(), t),
				f1(P1.Z(), P2.Z(), P3.Z(), P4.Z(), t));
	}

	Vector3 Beze(Vector3 P1, Vector3 P2, Vector3 P3, Vector3 P4, double t)
	{
		return Vector3(
			f2(P1.X(), P2.X(), P3.X(), P4.X(), t),
			f2(P1.Y(), P2.Y(), P3.Y(), P4.Y(), t),
			f2(P1.Z(), P2.Z(), P3.Z(), P4.Z(), t));
	}

	double f1(double p1, double p4, double r1, double r4, double t)
	{
		return p1 * (2 * t * t * t - 3 * t * t + 1) + p4 * (-2 * t * t * t + 3 * t * t) + r1 * (t * t * t - 2 * t * t + t) + r4 * (t * t * t - t * t);
	}

	double f2(double p1, double p2, double p3, double p4, double t)
	{
		return (1 - t) * (1 - t) * (1 - t) * p1 + 3 * t * (1 - t) * (1 - t) * p2 + 3 * t * t * (1 - t) * p3 + t * t * t * p4;
	}
};

bool cam_on_a_rocket = false;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{
		if(!cam_on_a_rocket)
			lookPoint.setCoords(0, 0, 0);
		else
		{ 
			lookPoint.setCoords(Rocket.pos.X(), Rocket.pos.Y(), Rocket.pos.Z());
			lookPoint = lookPoint + Rocket.direction * 0.5;
		}

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		pos = pos + lookPoint;

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры

void Particle::Draw()
{
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize((LifeTime / maxLifeTime * 500 + 10 + Rocket.speed/2) / (camera.pos - pos).length());
	glColor4d(color, color, color, 1 - LifeTime / maxLifeTime);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_POINT_SMOOTH);
	glBegin(GL_POINTS);
	glVertex3dv(pos.toArray());
	glEnd();
}

//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(10000, -10000, 5000);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света

void Batterfly::Draw(int numb)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	s[0].UseShader();
	int location = glGetUniformLocationARB(s[0].program, "iTexture0");
	if (location > -1)
	{
		glActiveTexture(GL_TEXTURE1);
		BTex.bindTexture();
		glUniform1iARB(location, 1);

	}
	location = glGetUniformLocationARB(s[0].program, "iTexture1");
	if (location > -1)
	{
		glActiveTexture(GL_TEXTURE2);
		BNM.bindTexture();
		glUniform1iARB(location, 2);

	}
	location = glGetUniformLocationARB(s[0].program, "iLightPos");
	if (location > -1)
	{
		glUniform3fARB(location, light.pos.X(), light.pos.Y(), light.pos.Z());
	}

	location = glGetUniformLocationARB(s[0].program, "iCamPos");
	if (location > -1)
	{
		glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());
	}
	location = glGetUniformLocationARB(s[0].program, "iModelViewMatrix");
	if (location > -1)
	{
		float mv_matrix[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mv_matrix);
		glUniformMatrix4fv(location, 1, false, mv_matrix);
	}

	PUSH;
	glNormal3d(0, 0, 1);
	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(angle, 0, 0, 1);
	glRotated((sin((Time + numb) * 15) + 1) * 30, 1, 0, 0);
	glScaled(0.03, 0.03, 0.03);
	glBegin(GL_QUADS);

	glTexCoord2d(1, 1);
	glVertex3d(0.5, 1, 0);

	glTexCoord2d(1, 0);
	glVertex3d(0.5, 0, 0);

	glTexCoord2d(0, 0);
	glVertex3d(-0.5, 0, 0);

	glTexCoord2d(0, 1);
	glVertex3d(-0.5, 1, 0);

	glEnd();

	POP;

	PUSH;

	glTranslated(pos.X(), pos.Y(), pos.Z());
	glRotated(angle, 0, 0, 1);
	glRotated(-(sin((Time + numb) * 15) + 1) * 30, 1, 0, 0);
	glScaled(0.03, 0.03, 0.03);
	glBegin(GL_QUADS);

	glTexCoord2d(1, 1);
	glVertex3d(0.5, -1, 0);

	glTexCoord2d(1, 0);
	glVertex3d(0.5, 0, 0);

	glTexCoord2d(0, 0);
	glVertex3d(-0.5, 0, 0);

	glTexCoord2d(0, 1);
	glVertex3d(-0.5, -1, 0);

	glEnd();
	POP;
	Shader::DontUseShaders();
}

//старые координаты мыши
int mouseX = 0, mouseY = 0;

float offsetX = 0, offsetY = 0;
float zoom=1;


//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	if (!OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON) && !replacement)
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y, 60, ogl->aspect);

		if (!launched)
			for (double hight = 1; hight < 2; hight += 0.1)
			{
				double k = 0, x = 0, y = 0;
				double z = hight;

				if (r.direction.Z() == 0)
					k = 0;
				else
					k = (z - r.origin.Z()) / r.direction.Z();

				x = k * r.direction.X() + r.origin.X();
				y = k * r.direction.Y() + r.origin.Y();

				if ((x <= Button.pos.X() + 0.630 && x >= Button.pos.X() - 0.630) && (y <= Button.pos.Y() + 0.630 && y >= Button.pos.Y() - 0.630))
				{
					launched = true;
					cam_on_a_rocket = true;
					Button.dir = -1;
				}
			}

	}
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}	   

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	/*if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}*/

	if (key == 'Q')
		Time = 0;

	if (key == 'C')
		cam_on_a_rocket = !cam_on_a_rocket;

	if (key == 'X' && launched)
	{ 
		Rocket.reset();
		launched = false;
		cam_on_a_rocket = false;
		replacement = true;
		Platform.dir = -1;
	}


	if (launched)
	{
		if (OpenGL::isKeyPressed(32))
		{
			if (Rocket.speed < 50)
				Rocket.speed += Rocket.speed / 40;
		}

		if (OpenGL::isKeyPressed(17))
		{
			if (Rocket.speed >= 1.1)
				Rocket.speed -= Rocket.speed / 40;
		}

		if(Rocket.speed >= 1)
		{
			if (OpenGL::isKeyPressed('A'))
			{
				Rocket.TurnAD(1, OpenGL::isKeyPressed(16));
			}
			if (OpenGL::isKeyPressed('D'))
			{
				Rocket.TurnAD(-1, OpenGL::isKeyPressed(16));
			}
			if (OpenGL::isKeyPressed('W'))
			{
				Rocket.TurnWS(1);
			}
			if (OpenGL::isKeyPressed('S'))
			{
				Rocket.TurnWS(-1);
			}
		}
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{

}

vector<std::string> facesTwo =
{
	"SkyBox//skyboxsun5deg2//skyrender0001.bmp",
	"SkyBox//skyboxsun5deg2//skyrender0004.bmp",
	"SkyBox//skyboxsun5deg2//skyrender0003.bmp",
	"SkyBox//skyboxsun5deg2//skyrender0006.bmp",
	"SkyBox//skyboxsun5deg2//skyrender0005.bmp",
	"SkyBox//skyboxsun5deg2//skyrender0002.bmp"
};

Vector3 skyboxV[6][4] = {
	{{1, 1, -1}, {1, -1, -1}, {1, -1, 1}, {1, 1, 1}},
	{{-1, -1, -1}, {-1, 1, -1}, {-1, 1, 1}, {-1, -1, 1}},
	{{-1, 1, 1}, {1, 1, 1}, {1, -1, 1}, {-1, -1, 1}},
	{{-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}},
	{{-1, 1, -1}, {1, 1, -1}, {1, 1, 1}, {-1, 1, 1}},
	{{1, -1, -1}, {-1, -1, -1}, {-1, -1, 1}, {1, -1, 1}}
};

Texture skyboxT[6];

ObjFile land;
Texture landTex;

Vector3 Pnts1[4] = { {1.5,0.5,3}, {0.8,-0.5,2.5}, {0.7, 0.6,2.7}, {1.5,0.5,3} };
Vector3 Pnts2[4] = { {-1.3,0.2,2.5}, {1,-1.8,2}, {-1.2, -0.3,1.7}, {-1.3,0.2,2.5} };
Vector3 Pnts3[4] = { {0.8,-1,1.5}, {1.4,-0.6,1.8}, {-0.7, 1.5,3.8}, {0.8,-1,1.5} };
Vector3 Pnts4[4] = { {-1.3,1.3,1.9}, {-0.8,-1.5,1.4}, {1.3, 0.6,2.7}, {-1.3,1.3,1.9} };
Vector3 Pnts5[4] = { {1.1,1.2,3.1}, {-1.5,-0.5,2.5}, {-0.7, -1.6,2.8}, {1.1,1.2,3.1} };

Batterfly batt[5] = { Batterfly(Pnts1), Batterfly(Pnts2), Batterfly(Pnts3), Batterfly(Pnts4), Batterfly(Pnts5)};

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	
	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	const GLfloat outside_light[4] = { 1, 1, 1, 100.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, outside_light);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	batt[0].BTex.loadTextureFromFile("textures\\butter.bmp");
	batt[0].BNM.loadTextureFromFile("textures\\butterNM.bmp");
	for (int i = 1; i < 5; i++)
	{
		batt[i].BTex = batt[0].BTex;
		batt[i].BNM = batt[0].BNM;
	}

	FlameTex.loadTextureFromFile("textures\\texture3.bmp"); //  загрузка текстуры из файла

	for (int i = 0; i < 6; i++)
	{
		skyboxT[i].loadTextureFromFile(facesTwo[i].c_str());
	}

	s[0].VshaderFileName = "shaders\\btf.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\btf.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	glActiveTexture(GL_TEXTURE0);

	loadModel("models\\Rocket_redy.obj", &Rocket.rocket);
	Rocket.rocketTex.loadTextureFromFile("textures//RocketTex.bmp");
	Rocket.rocketTex.bindTexture();

	loadModel("models\\Button.obj", &Button.button);
	Button.buttonTex.loadTextureFromFile("textures//ButtonTex.bmp");
	Button.buttonTex.bindTexture();

	loadModel("models\\Land.obj", &land);
	landTex.loadTextureFromFile("textures//LandTex.bmp");
	landTex.bindTexture();


	tick_n = GetTickCount();
	tick_o = tick_n;


}

void DrowEasySkyBox()
{
	glDisable(GL_DEPTH_TEST);
	glColor3d(1, 1, 1);

	Vector3 pos = camera.pos;

	for (int i = 0; i < 6; i++)
	{
		skyboxT[i].bindTexture();
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0);
		glVertex3dv((skyboxV[i][0] + pos).toArray());

		glTexCoord2d(1, 0);
		glVertex3dv((skyboxV[i][1] + pos).toArray());

		glTexCoord2d(1, 1);
		glVertex3dv((skyboxV[i][2] + pos).toArray());

		glTexCoord2d(0, 1);
		glVertex3dv((skyboxV[i][3] + pos).toArray());
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
}

void Render(OpenGL *ogl)
{   
	
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);


	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.2, 1. };
	GLfloat dif[] = { 0.51, 0.51, 0.51, 1. };
	GLfloat spec[] = { 0.8, 0.8, 0.8, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================

	DrowEasySkyBox();

	if (lightMode)
		glEnable(GL_LIGHTING);

	if(!launched)
		Rocket.Draw();
	else
		Rocket.Move(0.01);

	if (Button.Where() == 1 && Button.dir != 0)
		Button.dir = -1;
	if (Button.Where() == -1 && Button.dir != 0)
		Button.dir = 1;
	Button.Move();

	glPushMatrix();
	landTex.bindTexture();
	land.DrawObj();
	glPopMatrix();

	if (Platform.Where() == 1 && Platform.dir != 0)
		Platform.dir = -1;
	if (Platform.Where() == -1 && Platform.dir != 0)
		Platform.dir = 1;
	Platform.Move();

	Particles.Draw();


	for(int i = 0; i < 5; i++)
		batt[i].Move(i*20);

	Shader::DontUseShaders();

}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);

	GuiTextRectangle rec;

	rec.setSize(300, 260);
	rec.setPosition(5, ogl->getHeight() - 260 - 10);

	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "W/S - поворот по вертикали" << std::endl;
	ss << "A/D - поворот вокруг глобальной оси Z" << std::endl;
	ss << "SHIFT+A/D - поворот вокруг главной оси" << std::endl;
	ss << "Space/CTRL - увеличение/снижение тяги" << std::endl;
	ss << "С - переключение камеры на " << (cam_on_a_rocket ? "поверхность" : "ракету") << std::endl;
	ss << "X - уничтожение ракеты" << std::endl;
	ss << "Ракета запущена - " << (launched ? "Да":"Нет") << std::endl;
	ss << "Тяга = " << 60000 * Rocket.speed << std::endl;
	ss << "Скорость = " << (Rocket.speed >= 1 ? Rocket.speed : 0)/5<< "км/с" << (Rocket.speed < 1 ?" (недостаточно тяги)":"") << std::endl;
	ss << "Высота = " << Rocket.pos.Z()/2.2 << "км" << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	Shader::DontUseShaders(); 
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(5, newH - 260 - 10);
}

