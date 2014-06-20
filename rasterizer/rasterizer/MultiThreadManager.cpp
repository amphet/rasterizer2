#include "MultiThreadManager.h"



CMultiThreadManager::CMultiThreadManager()
{
	m_nThreadNum = DEFAULT_THREAD_NUM;
	m_bThreadRun = true;
	InitializeCriticalSection(&m_CS);

}


CMultiThreadManager::~CMultiThreadManager()
{
	//if(m_bThreadRun)  
	//DeleteWorkThreads();

	DeleteCriticalSection(&m_CS);

}


void CMultiThreadManager::CreateWorkThreads(int numofthreads)//numofthreas 개 thread 생성
{
	if (numofthreads)	m_nThreadNum = numofthreads;

	printf("create %d threads(Rasterizer)...\n", m_nThreadNum);
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
		m_pThreads[i] = std::thread(&CMultiThreadManager::ThreadFunction, this, i);
		m_pThreads[i].detach();
	}
}
void CMultiThreadManager::DeleteWorkThreads()//스레드 모두 삭제
{
	printf("RASTERIZER THREAD DELETE START\n");
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
	printf("RASTERIZER THREAD DELETE END\n");


}
void CMultiThreadManager::SetTileSize(int size){//binning크기변경
	m_nTileSize = size;
}

void CMultiThreadManager::InvokeWorkThreads(char mode){//thread wakeup

	m_nStage = mode;

	

	switch (m_nStage)
	{
	case VERTEX_SHADER:
		break;
	case PIXEL_SHADER:
		break;
	case RASTERIZER_BY_FACE:
		for (int i = 0; i<480; i++)
			for (int j = 0; j < 640; j++)
				m_fZBuffer[i][j] = FLT_MAX;
		break;
	default:
		printf("??????");
		break;

	}
	//signal to threads
	m_nWorkID = -1;
	for (int i = 0; i < m_nThreadNum; i++)
		SetEvent(m_pEvents[i]);


}
void CMultiThreadManager::WaitForWorkThreads(){

	WaitForMultipleObjects(m_nThreadNum, m_pEndEvents, true, INFINITE);
	
}

inline int myRound(const float a) { return int(a + 0.5); }




void CMultiThreadManager::ThreadFunction(int id){//스레드 함수

	printf("Thread %d ready...\n", id);

	CRasterizer myScanLineRasterizer;
	myScanLineRasterizer.SetScreenBufferPtr(m_pScreenBuffer);

	bool hit;
	int VertexFrom, VertexTo;
	int FaceFrom, FaceTo;
	int cnt;

	VertexFrom = VertexTo = 0;

	for (int i = 0; i < id + 1; i++)
	{
		VertexFrom = VertexTo;
		cnt = m_nNumOfVertice / m_nThreadNum;
		if (i < m_nNumOfVertice % m_nThreadNum) cnt++;
		VertexTo = VertexFrom + cnt;

	}

	FaceFrom = FaceTo = 0;

	for (int i = 0; i < id + 1; i++)
	{
		FaceFrom = FaceTo;
		cnt = m_nNumOfFace / m_nThreadNum;
		if (i < m_nNumOfFace % m_nThreadNum) cnt++;
		FaceTo = FaceFrom + cnt;

	}
	


	while (1)
	{
		//wait for thread Invoke
		WaitForSingleObject(m_pEvents[id], INFINITE);
		
		
		if (m_bThreadRun == false){
			printf("kill thread %d...\n", id);
			break;
		}
		//call each Shader Function
		switch (m_nStage)
		{
		case VERTEX_SHADER:
			VertexShader(VertexFrom, VertexTo);
			break;
		case PIXEL_SHADER:
			PixelShader(FaceFrom, FaceTo);
			break;
		case RASTERIZER_BY_FACE:
			Rasterizer_by_Face(&myScanLineRasterizer, id, FaceTo);
			break;
		}
		//signal END to manager
		SetEvent(m_pEndEvents[id]);
	}
	//printf("Thread Kill(ID : %d) \n", id);
	SetEvent(m_pKillEvents[id]);
}

void CMultiThreadManager::Rasterizer_by_Face(CRasterizer* myRasterizer, int from, int to){//rasterizer 함수
	int myWorkID;
	int TileXmin, TileXmax, TileYmin, TileYmax;
	int a, b, c;
	//int XSizeOfBin = SCREEN_WIDTH / m_nBinColCnt;
	//int YSizeOfBin = SCREEN_HEIGHT / m_nBinRowCnt;

	_POINT3D p1, p2, p3, N;
	_POINT3D v1, v2;
	_POINT3D Centroid, L;
	float maxX, minX, maxY, minY, maxZ, minZ;
	//LARGE_INTEGER freq;
	//LARGE_INTEGER stop;
	//LARGE_INTEGER start;

	//QueryPerformanceFrequency(&freq);
	//QueryPerformanceCounter(&start);

	while (1)
	{


		myWorkID = _InterlockedIncrement(&m_nWorkID);

		if (myWorkID >= m_nNumOfFace)
		{
			break;
		}

		int i = myWorkID;



		//if (myFinger >= m_nNumOfFace)//if (myFinger >= m_nBinRowCnt*m_nBinColCnt)
		{
			//break;
		}


		/*int myRow = myFinger / m_nBinColCnt;
		int myCol = myFinger % m_nBinColCnt;

		TileXmin = myCol * XSizeOfBin;
		TileYmin = myRow * YSizeOfBin;
		TileXmax = TileXmin + XSizeOfBin;
		TileYmax = TileYmin + YSizeOfBin;*/
		//myRasterizer->SetBinningSize(0, 0, 640, 480);

		//for (int i = 0; i < m_nNumOfFace; i++)
		//int i = myWorkID;
		a = pFace[myWorkID].VertexIndex[0];
		b = pFace[myWorkID].VertexIndex[1];
		c = pFace[myWorkID].VertexIndex[2];





		p1.x = m_pVertex[a].x;
		p1.y = m_pVertex[a].y;
		p1.z = m_pVertex[a].z;

		p2.x = m_pVertex[b].x;
		p2.y = m_pVertex[b].y;
		p2.z = m_pVertex[b].z;

		p3.x = m_pVertex[c].x;
		p3.y = m_pVertex[c].y;
		p3.z = m_pVertex[c].z;



		v1.x = p2.x - p1.x;
		v1.y = p2.y - p1.y;
		v1.z = p2.z - p1.z;
		v1.w = 1;

		v2.x = p3.x - p1.x;
		v2.y = p3.y - p1.y;
		v2.z = p3.z - p1.z;
		v2.w = 1;

		Cross_Vector3D(N, v1, v2);


		//hidden face culling
		if (N.z >= 0){
			pFace[myWorkID].bisBackFace = true;

			continue;
		}
		else{
			Normalize_Vector3D(N);
			pFace[myWorkID].bisBackFace = false;
			pFace[myWorkID].NormVec.x = N.x;
			pFace[myWorkID].NormVec.y = N.y;
			pFace[myWorkID].NormVec.z = N.z;
			pFace[myWorkID].NormVec.w = N.w;
		}


		maxX = max(p1.x, p2.x);
		maxX = max(maxX, p3.x);

		maxY = max(p1.y, p2.y);
		maxY = max(maxY, p3.y);

		minX = min(p1.x, p2.x);
		minX = min(minX, p3.x);

		minY = min(p1.y, p2.y);
		minY = min(minY, p3.y);

		minZ = min(p1.z, p2.z);
		minZ = min(minZ, p3.z);

		maxZ = max(p1.z, p2.z);
		maxZ = max(maxZ, p3.z);

		if (0 > maxX || minX >= SCREEN_WIDTH || 0 > maxY || minY >= SCREEN_HEIGHT || g_lpCamera3D->near_clip_z > maxZ || g_lpCamera3D->far_clip_z <= minZ)
		{
			pFace[myWorkID].bisBackFace = true;//outside of screen

			continue;
		}

		pFace[myWorkID].fMinX = minX;
		pFace[myWorkID].fMaxX = maxX;
		pFace[myWorkID].fMinY = minY;
		pFace[myWorkID].fMaxY = maxY;

		Centroid.x = (p1.x + p2.x + p3.x) / 3;
		Centroid.y = (p1.y + p2.y + p3.y) / 3;
		Centroid.z = (p1.z + p2.z + p3.z) / 3;

		Sub_Vector3D(L, g_Lighting, Centroid);
		Normalize_Vector3D(L);

		float dist = sqrt((g_Lighting.x - Centroid.x)*(g_Lighting.x - Centroid.x)
			+ (g_Lighting.y - Centroid.y)*(g_Lighting.y - Centroid.y)
			+ (g_Lighting.z - Centroid.z)*(g_Lighting.z - Centroid.z)
			);

		//teapot(0.45,0.9)
		float ar = 114.75f;//ka*ia;

		float dr = 100.5f*(N.x*L.x + N.y*L.y + N.z*L.z);//kd*id*(N.x*L.x + N.y*L.y + N.z*L.z);


		float light = (ar + dr) / (0 + 0.003*dist);
		int colour = myRound(light);
		if (colour > 255) colour = 255;
		else if (colour < 0) colour = 0;
		pFace[myWorkID].nColor = (char)colour;

		{

			if (m_pFace[i].bisBackFace) continue;

			//if (TileXmin > pFace[i].fMaxX || pFace[i].fMinX >= TileXmax || TileYmin > pFace[i].fMaxY || pFace[i].fMinY >= TileYmax) continue;
			//if (0 > pFace[i].fMaxX || pFace[i].fMinX >= 640 || 0 > pFace[i].fMaxY || pFace[i].fMinY >= 480) continue;

			a = m_pFace[i].VertexIndex[0];//objData->faceList[i]->vertex_index[0];
			b = m_pFace[i].VertexIndex[1];//objData->faceList[i]->vertex_index[1];
			c = m_pFace[i].VertexIndex[2];//objData->faceList[i]->vertex_index[2];

			myRasterizer->Launch(m_pVertex[a], m_pVertex[b], m_pVertex[c], m_pFace[i].NormVec, m_fZBuffer, m_pFace[i].nColor);
		}

	}
	//QueryPerformanceCounter(&stop);
	//__int64 hi = (stop.QuadPart - start.QuadPart) / (freq.QuadPart / 1000000.0f);
	//printf("%d\n", hi);

}


void CMultiThreadManager::VertexShader(int from, int to){
	_POINT3D p, cp;
	/*_MATRIX3D mtrxCamera;

	mtrxCamera.m00 = g_lpCamera3D->mcam.m00;
	mtrxCamera.m01 = g_lpCamera3D->mcam.m01;
	mtrxCamera.m02 = g_lpCamera3D->mcam.m02;
	mtrxCamera.m03 = g_lpCamera3D->mcam.m03;

	mtrxCamera.m10 = g_lpCamera3D->mcam.m10;
	mtrxCamera.m11 = g_lpCamera3D->mcam.m11;
	mtrxCamera.m12 = g_lpCamera3D->mcam.m12;
	mtrxCamera.m13 = g_lpCamera3D->mcam.m13;

	mtrxCamera.m20 = g_lpCamera3D->mcam.m20;
	mtrxCamera.m21 = g_lpCamera3D->mcam.m21;
	mtrxCamera.m22 = g_lpCamera3D->mcam.m22;
	mtrxCamera.m23 = g_lpCamera3D->mcam.m23;

	mtrxCamera.m30 = g_lpCamera3D->mcam.m30;
	mtrxCamera.m31 = g_lpCamera3D->mcam.m31;
	mtrxCamera.m32 = g_lpCamera3D->mcam.m32;
	mtrxCamera.m33 = g_lpCamera3D->mcam.m33;

	float zoom = g_lpCamera3D->zoom;
	float viewport_center_x, viewport_center_y;
	viewport_center_x = g_lpCamera3D->viewport_center_x;
	viewport_center_y = g_lpCamera3D->viewport_center_y;


	for (int i = from; i < to; i++)
	{

	cp.x = pVertice[i].x;
	cp.y = pVertice[i].y;
	cp.z = pVertice[i].z;
	cp.w = 0;

	Mult_VM3D(p, cp, mtrxCamera);//g_lpCamera3D->mcam);// );
	float z = p.z;

	if (z > 1.0f) z = zoom / z;
	else z = zoom;

	p.x = p.x * z + viewport_center_x;
	p.y = -p.y * z + viewport_center_y;

	//p.x = p.x + g_lpCamera3D->viewport_center_x;
	//p.y = -p.y + g_lpCamera3D->viewport_center_y;

	m_pVertex[i].x = p.x;
	m_pVertex[i].y = p.y;
	m_pVertex[i].z = p.z;
	m_pVertex[i].w = p.w;

	}*/

	//Work Thread Function VertexShader()
	for (int i = from; i < to; i++)
	{
		cp.x = pVertice[i].x;//pVertices are original vertices from File
		cp.y = pVertice[i].y;
		cp.z = pVertice[i].z;
		cp.w = 0;

		Mult_VM3D(p, cp, g_lpCamera3D->mcam);// );
		float z = p.z;

		if (z > 1.0f) z = g_lpCamera3D->zoom / z;
		else z = g_lpCamera3D->zoom;

		p.x = p.x * z + g_lpCamera3D->viewport_center_x;
		p.y = -p.y * z + g_lpCamera3D->viewport_center_y;
		
		m_pVertex[i].x = p.x;//m_pVertex is ptr to CVertex.m_pPoints
		m_pVertex[i].y = p.y;
		m_pVertex[i].z = p.z;
		m_pVertex[i].w = p.w;

	}

}
void CMultiThreadManager::PixelShader(int from, int to){
	int myFinger, a, b, c;
	_POINT3D p1, p2, p3, N;
	_POINT3D v1, v2;
	_POINT3D Centroid, L;
	float maxX, minX, maxY, minY;
	//myFinger = from;
	for (myFinger = from; myFinger < to; myFinger++)
		//while (1)//myFinger < m_nNumOfFace)
	{

		//EnterCriticalSection(&m_CS);
		//myFinger = m_nWorkID;
		//m_nWorkID++;
		//LeaveCriticalSection(&m_CS);

		//if (myFinger >= m_nNumOfFace)
		//	{
		//		break;
		//	}
		a = pFace[myFinger].VertexIndex[0];
		b = pFace[myFinger].VertexIndex[1];
		c = pFace[myFinger].VertexIndex[2];




		p1.x = m_pVertex[a].x;
		p1.y = m_pVertex[a].y;
		p1.z = m_pVertex[a].z;

		p2.x = m_pVertex[b].x;
		p2.y = m_pVertex[b].y;
		p2.z = m_pVertex[b].z;

		p3.x = m_pVertex[c].x;
		p3.y = m_pVertex[c].y;
		p3.z = m_pVertex[c].z;



		v1.x = p2.x - p1.x;
		v1.y = p2.y - p1.y;
		v1.z = p2.z - p1.z;
		v1.w = 1;

		v2.x = p3.x - p1.x;
		v2.y = p3.y - p1.y;
		v2.z = p3.z - p1.z;
		v2.w = 1;

		Cross_Vector3D(N, v1, v2);


		//hidden face culling
		if (N.z >= 0){
			pFace[myFinger].bisBackFace = true;

			continue;
		}
		else{
			Normalize_Vector3D(N);
			pFace[myFinger].bisBackFace = false;
			pFace[myFinger].NormVec.x = N.x;
			pFace[myFinger].NormVec.y = N.y;
			pFace[myFinger].NormVec.z = N.z;
			pFace[myFinger].NormVec.w = N.w;
		}


		maxX = max(p1.x, p2.x);
		maxX = max(maxX, p3.x);

		maxY = max(p1.y, p2.y);
		maxY = max(maxY, p3.y);

		minX = min(p1.x, p2.x);
		minX = min(minX, p3.x);

		minY = min(p1.y, p2.y);
		minY = min(minY, p3.y);

		if (0 > maxX || minX >= SCREEN_WIDTH || 0 > maxY || minY >= SCREEN_HEIGHT)
		{
			pFace[myFinger].bisBackFace = true;//outside of screen

			continue;
		}

		pFace[myFinger].fMinX = minX;
		pFace[myFinger].fMaxX = maxX;
		pFace[myFinger].fMinY = minY;
		pFace[myFinger].fMaxY = maxY;

		Centroid.x = (p1.x + p2.x + p3.x) / 3;
		Centroid.y = (p1.y + p2.y + p3.y) / 3;
		Centroid.z = (p1.z + p2.z + p3.z) / 3;

		Sub_Vector3D(L, g_Lighting, Centroid);
		Normalize_Vector3D(L);

		float dist = sqrt((g_Lighting.x - Centroid.x)*(g_Lighting.x - Centroid.x)
			+ (g_Lighting.y - Centroid.y)*(g_Lighting.y - Centroid.y)
			+ (g_Lighting.z - Centroid.z)*(g_Lighting.z - Centroid.z)
			);

		//teapot(0.45,0.9)
		//float ka = 0.45;
		//float kd = 0.9;

		//int ia = 255;
		//int id = 255;

		float ar = 114.75f;//ka*ia;

		float dr = 229.5f*(N.x*L.x + N.y*L.y + N.z*L.z);//kd*id*(N.x*L.x + N.y*L.y + N.z*L.z);


		float light = (ar + dr) / (0 + 0.003*dist);
		int colour = myRound(light);
		if (colour > 255) colour = 255;
		else if (colour < 0) colour = 0;
		pFace[myFinger].nColor = (char)colour;


	}
}

void CMultiThreadManager::Rasterizer_by_Tile(){}
