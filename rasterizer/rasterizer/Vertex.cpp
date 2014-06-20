#include "Vertex.h"
#include <stdio.h>
#include <ppl.h>
#include <array>



CVertex::CVertex()
{
	}


CVertex::~CVertex()
{
	delete m_pPoints;
	
}

bool CVertex::Setup(void* something)
{
	WorldMat.v[0][0] = 1;
	WorldMat.v[0][1] = 0;
	WorldMat.v[0][2] = 0;
	WorldMat.v[0][3] = 0;

	WorldMat.v[1][0] = 0;
	WorldMat.v[1][1] = 1;
	WorldMat.v[1][2] = 0;
	WorldMat.v[1][3] = 0;

	WorldMat.v[2][0] = 0;
	WorldMat.v[2][1] = 0;
	WorldMat.v[2][2] = 1;
	WorldMat.v[2][3] = 0;

	WorldMat.v[3][0] = 0;
	WorldMat.v[3][1] = 0;
	WorldMat.v[3][2] = 0;
	WorldMat.v[3][3] = 1;


	
	return true;
}
bool CVertex::startup(int Vertexcnt)
{
	m_nVertexCount = Vertexcnt;
	
	m_pPoints = new _POINT3D[m_nVertexCount];

	

	return true;
}

void CVertex::SetLight(float x, float y, float z)
{
	g_Lighting.x = x;
	g_Lighting.y = y;
	//teapot g_Lighting.z = 100;
	g_Lighting.z = z;
	g_Lighting.w = 0;
}
void CVertex::Light()
{
	_POINT3D p;
	g_Lighting.x = 20;
	g_Lighting.y = 80;
	g_Lighting.z = 0;
	g_Lighting.w = 0;

	Mult_VM3D(p, g_Lighting, g_lpCamera3D->mcam);
	g_Lighting.x = p.x;
	g_Lighting.y = p.y;
	g_Lighting.z = p.z;
	g_Lighting.w = p.w;
	float z = g_Lighting.z;
	if (z > 1.0f)
	{
		z = g_lpCamera3D->zoom / z;
		g_Lighting.x *= z;
		g_Lighting.y *= z;
	}
	else
	{
		z = g_lpCamera3D->zoom;
		g_Lighting.x *= z;
		g_Lighting.y *= z;
	}
	
	
	g_Lighting.x = g_Lighting.x+ g_lpCamera3D->viewport_center_x;
	g_Lighting.y = -g_Lighting.y + g_lpCamera3D->viewport_center_y;

	//printf("Light : %f %f %f\n", g_Lighting.x, g_Lighting.y, g_Lighting.z);
	

	
}