#pragma once
#pragma once
#include "Global.h"
#include <thread>

#define DONT_CHANGE 0

#define DEFAULT_THREAD_NUM 1

class CTransformThread
{
public:
	CTransformThread();
	~CTransformThread();


	std::thread m_pThreads[4];
	HANDLE *m_pEvents;
	HANDLE *m_pEndEvents;
	HANDLE *m_pKillEvents;
	bool m_bThreadRun;
	
	CRITICAL_SECTION m_CS;

	int m_nThreadNum;
	int m_nVerticeNum;

	void SetVerticeNum(int i){
		m_nVerticeNum = i;
	}

	void CreateWorkThreads(int numofthreads);//numofthreas �� thread ����
	void DeleteWorkThreads();//������ ��� ����

	_POINT3D* m_pVertex;
	void SetVertexPtr(_POINT3D* ptr){ m_pVertex = ptr; }

	void InvokeWorkThreads();//signaling
	void WaitForWorkThreads();//������ �۾� ���������� ���
	void ThreadFunction(int id);
};

