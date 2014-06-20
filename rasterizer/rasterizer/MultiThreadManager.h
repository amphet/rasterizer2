
#pragma once
#include "Global.h"
#include <thread>
#include "Rasterizer.h"

#define DONT_CHANGE 0

#define DEFAULT_THREAD_NUM 1
#define VERTEX_SHADER 0
#define PIXEL_SHADER 1
#define RASTERIZER_BY_FACE 2


class CMultiThreadManager
{
public:
	CMultiThreadManager();
	~CMultiThreadManager();

	std::thread m_pThreads[8];
	HANDLE *m_pEvents;
	HANDLE *m_pEndEvents;
	HANDLE *m_pKillEvents;
	bool m_bThreadRun;
	char m_nStage;

	CRITICAL_SECTION m_CS;




	MyFaceInfo *m_pFace;
	void SetFacePtr(MyFaceInfo *ptr){ m_pFace = ptr; }
	_POINT3D* m_pVertex;
	void SetVertexPtr(_POINT3D* ptr){ m_pVertex = ptr; }
	void SetNumOfFace(int numFace){ m_nNumOfFace = numFace; }

	float m_fZBuffer[480][640];
	byte(*m_pScreenBuffer)[640][3];
	void SetScreenBufferPtr(byte(*ptr)[640][3]){ m_pScreenBuffer = ptr; }



	long m_nWorkID;
	int m_nThreadNum;
	//int m_nBinRowCnt;//bin의 row 수

	//int m_nBinColCnt;//bin의 column 수

	int m_nTileSize;

	int m_nNumOfFace;
	int m_nNumOfVertice;
	void SetNumOfVertice(int numV){ m_nNumOfVertice = numV; }
	int tester;

	void CreateWorkThreads(int numofthreads);//numofthreas 개 thread 생성
	void DeleteWorkThreads();//스레드 모두 삭제
	void SetTileSize(int s);//binning 크기
	void InvokeWorkThreads(char mode);//signaling
	void WaitForWorkThreads();//스레드 작업 끝날때까지 대기
	void ThreadFunction(int id);
	void Rasterizer_by_Face(CRasterizer* myRasterizer, int from, int to);
	void Rasterizer_by_Tile();
	void VertexShader(int from, int to);
	void PixelShader(int from, int to);

};

