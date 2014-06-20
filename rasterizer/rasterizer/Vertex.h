#pragma once
#include "Global.h"
#include <thread>


class CVertex
{
public:
	CVertex();
	~CVertex();

	
	bool Setup(void* something);
	bool startup(int Vertexcnt);

	void SetLight(float x, float y,float z);
	void Light();


	//Vertex* m_pVertex;
	//char* m_pVertexTest;
	//_POINT3D* m_pNormVec;
	_POINT3D* m_pPoints;//transposed vertice
	

	
	//std::thread m_Threads[VERTEX_THREAD_NUM];
	//HANDLE hEvents[VERTEX_THREAD_NUM];
	//HANDLE hEndEvents[VERTEX_THREAD_NUM];
	//bool bThreadRun[VERTEX_THREAD_NUM];
	
	Matrix3D WorldMat;
	Matrix3D ViewMat;
	int m_nVertexCount;
	

};

