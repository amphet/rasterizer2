#include "TransformThread.h"


CTransformThread::CTransformThread()
{
	m_nThreadNum = DEFAULT_THREAD_NUM;
	m_bThreadRun = true;
	InitializeCriticalSection(&m_CS);

}


CTransformThread::~CTransformThread()
{
	DeleteCriticalSection(&m_CS);
}


void CTransformThread::CreateWorkThreads(int numofthreads)//numofthreas 개 thread 생성
{
	if (numofthreads)	m_nThreadNum = numofthreads;

	printf("create %d threads(Transform)...\n", m_nThreadNum);
	m_pEvents = new HANDLE[m_nThreadNum];
	m_pEndEvents = new HANDLE[m_nThreadNum];
	m_pKillEvents = new HANDLE[m_nThreadNum];

	m_bThreadRun = true;
	for (int i = 0; i < m_nThreadNum; i++)
	{

		m_pEvents[i] = CreateEvent(NULL, false, false, NULL);
		m_pEndEvents[i] = CreateEvent(NULL, false, false, NULL);
		m_pKillEvents[i] = CreateEvent(NULL, false, false, NULL);

	}
	//	m_pThreads = new std::thread[m_nThreadNum];

	for (int i = 0; i < m_nThreadNum; i++)
	{
		m_pThreads[i] = std::thread(&CTransformThread::ThreadFunction, this, i);
		m_pThreads[i].detach();
	}
}

void CTransformThread::DeleteWorkThreads()//스레드 모두 삭제
{
	printf("TRANSFORM THREAD DELETE START\n");
	m_bThreadRun = false;
	for (int i = 0; i < m_nThreadNum; i++)
	{

		SetEvent(m_pEvents[i]);
	}

	WaitForMultipleObjects(m_nThreadNum, m_pKillEvents, true, INFINITE);
	Sleep(500);
	for (int i = 0; i < m_nThreadNum; i++)
	{
		CloseHandle(m_pEvents[i]);
		CloseHandle(m_pEndEvents[i]);
		CloseHandle(m_pKillEvents[i]);
	}
	delete m_pEvents;
	delete m_pEndEvents;
	delete m_pKillEvents;
	printf("TRANSFORM THREAD DELETE END\n");


}


void CTransformThread::InvokeWorkThreads(){//thread wakeup
	for (int i = 0; i < m_nThreadNum; i++)
	{
		SetEvent(m_pEvents[i]);
	}
}


void CTransformThread::WaitForWorkThreads(){//thread작업끝날때까지 대기
	WaitForMultipleObjects(m_nThreadNum, m_pEndEvents, true, INFINITE);
}



void CTransformThread::ThreadFunction(int id){//스레드 함수

	printf("Transform Thread Created(ID : %d) \n", id);

	int from = 0;
	int end = 0;
	int cnt = m_nVerticeNum/m_nThreadNum;
	_POINT3D p, cp;


	for (int i = 0; i < id+1; i++)
	{
		from = end;

		end = from + cnt;
		if (i < m_nVerticeNum%m_nThreadNum) end++;
	}
	printf("transform Thread(%d) : %d - %d\n", id, from, end);



	while (1)
	{

		WaitForSingleObject(m_pEvents[id], INFINITE);
		
		if (m_bThreadRun == false)
		{
			printf("ready to kill thread %d...\n", id);
			break;
		}

		for (int i = from; i < end; i++)
		{
			
			
			cp.x = pVertice[i].x;
			cp.y = pVertice[i].y;
			cp.z = pVertice[i].z;
			cp.w = 0;

			Mult_VM3D(p, cp, g_lpCamera3D->mcam);
			float z = p.z;

			if (z > 1.0f) z = g_lpCamera3D->zoom / z;
			else z = g_lpCamera3D->zoom;

			p.x = p.x * z + +g_lpCamera3D->viewport_center_x;;
			p.y = -p.y * z + g_lpCamera3D->viewport_center_y;;

			//p.x = p.x + g_lpCamera3D->viewport_center_x;
			//p.y = -p.y + g_lpCamera3D->viewport_center_y;

			m_pVertex[i].x = p.x;
			m_pVertex[i].y = p.y;
			m_pVertex[i].z = p.z;
			m_pVertex[i].w = p.w;

			
		}
		SetEvent(m_pEndEvents[id]);

	}
	//delete myZBuffer;
	printf("Tranform Thread Kill(ID : %d) \n", id);

	SetEvent(m_pKillEvents[id]);
}
