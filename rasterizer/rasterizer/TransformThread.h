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

	void CreateWorkThreads(int numofthreads);//numofthreas 개 thread 생성
	void DeleteWorkThreads();//스레드 모두 삭제

	_POINT3D* m_pVertex;
	void SetVertexPtr(_POINT3D* ptr){ m_pVertex = ptr; }

	void InvokeWorkThreads();//signaling
	void WaitForWorkThreads();//스레드 작업 끝날때까지 대기
	void ThreadFunction(int id);
};

