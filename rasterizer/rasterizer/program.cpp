// Include Directive
// 


/****************************************************************




OBJLoader : example.txt
global.h : 전역변수, 구조체정의






*******************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <memory.h>
#include <math.h>

#include "glut.h"

#include <iostream>

#include "Global.h"
#include "Vertex.h"
#include "Tessellator.h"
#include "Rasterizer.h"
#include "PixelShader.h"
//#include "EdgeTable.h"
#include "MultiThreadManager.h"


// Constants Directive
//
//#define SCREEN_DIVIDE 40

// Global variables
//
typedef unsigned char byte;
byte g_pScreenImage[SCREEN_HEIGHT][SCREEN_WIDTH][COLOR_DEPTH];
//byte g_pScreenImage[SCREEN_HEIGHT][SCREEN_WIDTH][COLOR_DEPTH];

//float g_pZBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// Static variables
//
static GLdouble g_dZoomFactor = 1.0;
static GLint g_iHeight;


void drawLine(int x0, int y0, int xEnd, int yEnd);
void drawPixel(int x, int y);

void drawLineColor(int x0, int y0, float z0, int xEnd, int yEnd,float zEnd );
void drawPixelColor(int x, int y,int z);

inline int myRound(const float a) { return int(a + 0.5); }

void initilize();//초기화작업 = input assembler. 지금은 obj파일 읽어서 저장하는것만 집어넣음
void finalize();//종료작업. 동적할당 삭제처리 여기서 하면 될것
void ScreenBufferSet();
void Benchmark();
void PipelineCheck();

//float fMaxZ;
//float fMinZ;

bool bIsScanline;
bool bIsWireFrame;


CVertex transVertex;
CTessellator Tessellator;
CRasterizer Rasterizer;
CPixelShader PixelShader;
CMultiThreadManager ThreadManager;

//CEdgeTable *pEdgeTable[VERTEX_THREAD_NUM];

int g_cnt;

//unsigned char *pColor;




//
//typedef struct __EdgeEntryWrapper
//{
//	float fScanLineNum;
//	EdgeEntry_NEW *entry;
//}EdgeEntryWrapper_NEW;

// TODO: g_pScreenImage 메모리를 채우면 됩니다.
__int64 g_interval_VertexShader;
__int64 g_interval_PixelShader;
__int64 g_interval_RasterizeShader;
__int64 g_interval_GlutMonitor;
__int64 g_interval_minor;

void makeCheckImage(void)
{




	//printf("adadf\n");
	LARGE_INTEGER freq;
	LARGE_INTEGER stop;
	LARGE_INTEGER start;

	QueryPerformanceFrequency(&freq);
	
	memset(g_pScreenImage, -1, sizeof(g_pScreenImage));
	
	Update_Camera(g_lpCamera3D);
	
	QueryPerformanceCounter(&start);


	ThreadManager.InvokeWorkThreads(VERTEX_SHADER);
	
	transVertex.Light();//광원좌표변환
	ThreadManager.WaitForWorkThreads();
	QueryPerformanceCounter(&stop);
	g_interval_VertexShader += (stop.QuadPart - start.QuadPart) / (freq.QuadPart / 1000000.0f);
	

	QueryPerformanceCounter(&start);
	ThreadManager.InvokeWorkThreads(RASTERIZER_BY_FACE);
	ThreadManager.WaitForWorkThreads();
	QueryPerformanceCounter(&stop);
	g_interval_RasterizeShader += (stop.QuadPart - start.QuadPart) / (freq.QuadPart / 1000000.0f);


	
	
	
	

	

}



void initilize()
{
	//set dummy head
	char buff[20];
	gets(buff);

	objData = new objLoader();
	//objData->load("symphysis.obj");

	if (!(objData->load(buff)))
	{
		delete objData;
		exit(1);
	}

	//objData->load("brandenburggate.obj");
	//objData->load("bone.obj");
//	objData->load("roi.obj");

	m_ptMouse.x = 320;
	m_ptMouse.y = 240;
	
	bIsScanline = false;
	bIsWireFrame = false;
	printf("hello");

	
	pFace = new MyFaceInfo[objData->faceCount];
	
	for (int i = 0; i < objData->faceCount; i++)
	{
		pFace[i].VertexIndex[0] = objData->faceList[i]->vertex_index[0];
		pFace[i].VertexIndex[1] = objData->faceList[i]->vertex_index[1];
		pFace[i].VertexIndex[2] = objData->faceList[i]->vertex_index[2];
	}
	pVertice = new _POINT3D[objData->vertexCount];
	float k = 1.0f;
	//brandenburg gate : k = 60, y = -20;
	for (int i = 0; i < objData->vertexCount; i++)
	{
		pVertice[i].x = objData->vertexList[i]->e[0]*k;
		pVertice[i].y = objData->vertexList[i]->e[1] * k;// -20.0f;// +500.0f;
		pVertice[i].z = objData->vertexList[i]->e[2]*k;// +300;
	}
	transVertex.startup(objData->vertexCount);
	


	ThreadManager.SetFacePtr(pFace);
	
	ThreadManager.SetVertexPtr(transVertex.m_pPoints);
	ThreadManager.SetNumOfFace(objData->faceCount);
	ThreadManager.SetNumOfVertice(objData->vertexCount);
	ThreadManager.SetScreenBufferPtr(g_pScreenImage);
	ThreadManager.SetTileSize(8);
	
	ThreadManager.CreateWorkThreads(1);
	


	
	printf("# of vertice : %d\n", objData->vertexCount);
	printf("# of face : %d\n", objData->faceCount);
	printf("number key : # of threads\n");
	printf("0 : refresh screen");
	printf("b : start benchmark\n");
	printf("v : change binning info\n");
	printf("# of threads : %d\n", ThreadManager.m_nThreadNum);
	delete objData;
	objData = NULL;
	g_interval_GlutMonitor = 0;
	g_interval_PixelShader = 0;
	g_interval_RasterizeShader = 0;
	g_interval_VertexShader = 0;
	g_bThreadMode = BY_FACE;

	_POINT3D cam_pos, cam_dir, cam_target;
	Init_Vector3D(cam_pos, 0, 0, -100);//teapot!!
	Init_Vector3D(cam_dir, 0, 0, 0);
	Init_Vector3D(cam_target, 0, 0, 1);

	g_lpCamera3D = Init_Camera(cam_pos, cam_dir, cam_target, 5.0f, 5000.0f, 90.0f, 640, 480);
	


}


void finalize()
{
	ThreadManager.DeleteWorkThreads();


	delete pFace;
	delete pVertice;

	

	if (g_lpCamera3D != NULL)
	{
		delete g_lpCamera3D;
		g_lpCamera3D = NULL;
	}


}

void drawLineColor(int x0, int y0, float z0, int xEnd, int yEnd, float zEnd)
{
	int dx = xEnd - x0, dy = yEnd - y0, steps, k;
	float dz = zEnd - z0;

	float xIncrement, yIncrement, x = x0, y = y0;
	float zIncrement, z = z0;

	if (abs(dx) > abs(dy))
		steps = abs(dx);
	else
		steps = abs(dy);

	xIncrement = float(dx) / float(steps);
	yIncrement = float(dy) / float(steps);
	zIncrement = float(dz) / float(steps);


	drawPixelColor(myRound(x), myRound(y),z);
	for (k = 0; k < steps; k++)
	{
		x += xIncrement;
		y += yIncrement;
		z += zIncrement;
		drawPixelColor(myRound(x), myRound(y),z);
	}

}
void drawPixelColor(int x, int y, int z)
{
	if (0 <= x && x < SCREEN_WIDTH && 0 <= y&&y < SCREEN_HEIGHT)
	{
		//float hello = z;
		
		//hello = (hello / objData->faceCount) * 255;

		byte colour = (byte)z;
		g_pScreenImage[y][x][0] = colour;
		g_pScreenImage[y][x][1] = 0;
		g_pScreenImage[y][x][2] = 0;
	}

}



void drawLine(int x0, int y0, int xEnd,int yEnd)
{
	int dx = xEnd - x0, dy = yEnd - y0, steps, k;
	float xIncrement, yIncrement, x = x0, y = y0;

	if (abs(dx) > abs(dy))
		steps = abs(dx);
	else
		steps = abs(dy);

	xIncrement = float(dx) / float(steps);
	yIncrement = float(dy) / float(steps);
	
	drawPixel(myRound(x), myRound(y));
	for (k = 0; k < steps; k++)
	{
		x += xIncrement;
		y += yIncrement;
		drawPixel(myRound(x), myRound(y));
	}
			
}

void drawPixel(int x, int y)
{
	if (0 <= x && x < SCREEN_WIDTH && 0 <= y&&y < SCREEN_HEIGHT)
	{
		g_pScreenImage[y][x][0] = 0;
		g_pScreenImage[y][x][1] = 0;
		g_pScreenImage[y][x][2] = 0;
	}
}


//-----------------------------------------------------------------------------------------------------------------------
//
// DO NOT EDIT!!
//
//-----------------------------------------------------------------------------------------------------------------------
void init( void)
{	
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glShadeModel( GL_FLAT );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
}

void display( void)
{

	glClear( GL_COLOR_BUFFER_BIT );
	

	makeCheckImage();
	
	
	LARGE_INTEGER freq;
	LARGE_INTEGER stop;
	LARGE_INTEGER start;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	
	glRasterPos2i( 0, 480 );
	glPixelZoom( 1.f, -1.f );
	glDrawPixels( SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, g_pScreenImage );
	glFlush();
	QueryPerformanceCounter(&stop);
	g_interval_GlutMonitor += (stop.QuadPart - start.QuadPart) / (freq.QuadPart / 1000000.0f);
	//printf("BY GL : df\n\n", g_interval_GlutMonitor);

	
	
}

void reshape( int w, int h)
{
	glViewport( 0, 0,( GLsizei) w,( GLsizei) h );
	g_iHeight =( GLint) h;
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0,( GLdouble) w, 0.0,( GLdouble) h );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void motion( int x, int y)
{
	//printf( "[motion] x: %d, y: %d\n", x, y );


	Update_Mouse(x, y, g_lpCamera3D);

	
	/*
	static GLint screeny;

	screeny = g_iHeight -( GLint) y;
	glRasterPos2i( x, screeny );
	//glPixelZoom( g_dZoomFactor, g_dZoomFactor );
	glCopyPixels( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR );
	glPixelZoom( 1.0, 1.0 );*/
	display();
	//glFlush();
//	printf("motin end\n");


}

void keyboard( unsigned char key, int x, int y)
{
	//printf( "[keyboard] key: %c\n", key );
	
	HMODULE hCurrentModule = nullptr;
	_POINT3D v;
	int row, col;

	

	switch( key )
	{	

	case 'r':
	case 'R':
		g_dZoomFactor = 1.0;
		glutPostRedisplay();
		printf( "g_dZoomFactor reset to 1.0\n" );

		break;

	case 'z':
		g_dZoomFactor += 0.5;
		if ( g_dZoomFactor >= 5.0) 
		{
			g_dZoomFactor = 5.0;
		}
		printf( "g_dZoomFactor is now %4.1f\n", g_dZoomFactor );
		break;

	case 'Z':
		g_dZoomFactor -= 0.5;
		if ( g_dZoomFactor <= 0.5) 
		{
			g_dZoomFactor = 0.5;
		}
		printf( "g_dZoomFactor is now %4.1f\n", g_dZoomFactor );
		break;

	case 27:
		finalize();
		exit( 0 );
		break;
	case 'a':
		
		Cross_Vector3D(v, g_lpCamera3D->target, g_lpCamera3D->v);
		Add_Vector3D(g_lpCamera3D->pos, g_lpCamera3D->pos, v);
		//g_lpCamera3D->pos.x -= 1.0f;
		break;
	case 'd':
		//g_lpCamera3D->pos.x += 1.0f;
		
		Cross_Vector3D(v, g_lpCamera3D->target, g_lpCamera3D->v);
		Sub_Vector3D(g_lpCamera3D->pos, g_lpCamera3D->pos, v);
		break;
	case 'w':
		Add_Vector3D(g_lpCamera3D->pos, g_lpCamera3D->pos, g_lpCamera3D->target);
		break;
	case 's':
		Sub_Vector3D(g_lpCamera3D->pos, g_lpCamera3D->pos, g_lpCamera3D->target);
		break;
	case 'l':
		g_lpCamera3D->dir.x = 0.0f;
		g_lpCamera3D->dir.y = 0.0f;
		g_lpCamera3D->dir.z = 0.0f;
		g_lpCamera3D->target.x = 0.0f;
		g_lpCamera3D->target.y = 0.0f;
		g_lpCamera3D->target.z = 1.0f;
		break;
	case 'b':
		Benchmark();
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
		//printf("Sequential Pipeline Test(inside check)\n");
		printf("Use %d Rasterizer threads...\n", key - '0');
		ThreadManager.DeleteWorkThreads();
		ThreadManager.CreateWorkThreads(key - '0');
		bIsScanline = false;
		break;
	case '0':
		printf("Screen Refresh\n");
		PipelineCheck();
		break;
	case 'v':
		printf("Tile Size?\n");
		scanf("%d", &row);
		ThreadManager.DeleteWorkThreads();
		ThreadManager.SetTileSize(row);
		ThreadManager.CreateWorkThreads(DONT_CHANGE);
		
		break;
	case 'f':
		g_bThreadMode = !g_bThreadMode;
		if (g_bThreadMode == BY_FACE) printf("per face Rasterizing\n");
		if (g_bThreadMode == BY_TILE) printf("per TILE Rasterizing\n");

		break;
	default:
		break;
		
	}
	display();
}

int main( int argc, char** argv)
{
	/*
	_POINT3D p1, p2, p3;
	p1.x = 2; p1.y = 15;
	p2.x = 5; p2.y = 4;
	p3.x = 13; p3.y = 6;
	p1.z = p2.z = p3.z = p1.w = p2.w = p3.w = 1;
//	Rasterizer.Launch(p1, p2, p3, screen);
	int xsize = 30;
	int ysize = 20;
	for (int y = ysize; y > 0; y--)
	{
		for (int x = 1; x <= xsize; x++)
		{
			std::cout << screen[x][y] << " ";
		}
		std::cout << "\n";
	}
	while (1){}
	*/
	initilize();

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
	glutInitWindowSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	glutInitWindowPosition( 100, 100 );
	glutCreateWindow( argv[0] );
	init();
	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutMotionFunc( motion );
	glutMainLoop();

	return 0; 
}

void Benchmark()
{
	int i,k;
	float angleX = 0;
	float angleY = 0;
	float theta = 0;
	
	float slice = 0.02f;
	
	int iter = (2 * 3.141592) / slice;
	
	
	
	//g_lpCamera3D->pos.y -= 50;
	float dist =  Length_Vector3D(g_lpCamera3D->pos);
	
	printf("camdist : %f\n", dist);
	
	//dist = dist * 2;
	float distmap[3] = { dist * 3 / 4, dist, dist*2 };


	LARGE_INTEGER freq;
	LARGE_INTEGER stop;
	LARGE_INTEGER start;
	
	QueryPerformanceFrequency(&freq);
	
	

	__int64 mic_interval = 0;
	g_interval_GlutMonitor = 0;
	g_interval_PixelShader = 0;
	g_interval_RasterizeShader = 0;
	g_interval_VertexShader = 0;

	for (k = 0; k < 1; k++)
	{
		Init_Vector3D(g_lpCamera3D->pos, 0, 0, dist);//teapot!!
		//	Init_Vector3D(cam_pos, 0, 0, -10);//symphysis!!

		Init_Vector3D(g_lpCamera3D->dir, 0, 0, 0);
		Init_Vector3D(g_lpCamera3D->target, 0, 0, 1);
		
		

		for (i = 0; i < iter; i++)
		{
		
			//printf("i : %d\n", i);
			g_lpCamera3D->dir.y += slice;
			if (g_lpCamera3D->dir.y >= 2 * 3.141592)
			{
				g_lpCamera3D->dir.y -= 2 * 3.141592;
			}
			else if (g_lpCamera3D->dir.y <= -2 * 3.141592)
			{
				g_lpCamera3D->dir.y += 2 * 3.141592;
			}
			
			theta += slice;
			if (theta >= 2 * 3.141592)
			{
				theta -= 2 * 3.141592;
			}
			g_lpCamera3D->pos.x = -dist * sin(theta);
			g_lpCamera3D->pos.z = -dist * cos(theta);
			

			

			QueryPerformanceCounter(&start);
			display();
			QueryPerformanceCounter(&stop);

			//delay += (double)(stop.QuadPart - start.QuadPart) / 1000000.0f/ (double)freq.QuadPart;
			mic_interval += (stop.QuadPart - start.QuadPart) / (freq.QuadPart / 1000000.0f);
			
			//printf("%f %f %f\n", g_lpCamera3D->pos.x, g_lpCamera3D->pos.y, g_lpCamera3D->pos.z);
		}
		//dist = dist *0.5;
		/*
		g_lpCamera3D->pos.x = 0;
		g_lpCamera3D->pos.y = 0;
		g_lpCamera3D->pos.z = distmap[k];

		for (i = 0; i < iter; i++)
		{
			//printf("i : %d\n", i);
			g_lpCamera3D->dir.z += slice;
			if (g_lpCamera3D->dir.y >= 2 * 3.141592)
			{
				g_lpCamera3D->dir.y -= 2 * 3.141592;
			}
			else if (g_lpCamera3D->dir.y <= -2 * 3.141592)
			{
				g_lpCamera3D->dir.y += 2 * 3.141592;
			}
			//g_lpCamera3D->dir.x += 0.05;
			if (g_lpCamera3D->dir.z >= 2 * 3.141592)
			{
				g_lpCamera3D->dir.z -= 2 * 3.141592;
			}
			else if (g_lpCamera3D->dir.z <= -2 * 3.141592)
			{
				g_lpCamera3D->dir.z += 2 * 3.141592;
			}
			g_lpCamera3D->target.x = cos(g_lpCamera3D->dir.x)*sin(g_lpCamera3D->dir.y);
			g_lpCamera3D->target.z = cos(g_lpCamera3D->dir.x)*cos(g_lpCamera3D->dir.y);
			g_lpCamera3D->target.y = sin(-g_lpCamera3D->dir.x);
			theta += slice;
			if (theta >= 2 * 3.141592)
			{
				theta -= 2 * 3.141592;
			}
			g_lpCamera3D->pos.z = -distmap[k] * sin(theta);
			g_lpCamera3D->pos.y = -distmap[k] * cos(theta);

			dwStartTime = timeGetTime();
			display();
			dwTotal += timeGetTime() - dwStartTime;
			//printf("%f %f %f\n", g_lpCamera3D->pos.x, g_lpCamera3D->pos.y, g_lpCamera3D->pos.z);
		}
		*/	
	}
	printf("refresh : %dmicrosec to %d frame\n", iter,mic_interval);
	int avgTime = mic_interval / (iter*1);
	printf("avg frame process time : %d microsec\n", avgTime);
	double fps = 1000000.0 / avgTime;
	printf("fps : %f\n", fps);
	printf("avg VertexShader : %f\n", g_interval_VertexShader/(float)(iter*1));
	printf("avg Rasterizer : %f\n", g_interval_RasterizeShader / (float)(iter*1));
	printf("avg GLUTHW : %f\n", g_interval_GlutMonitor / (float)(iter*1));
	

	
	
	//std::cout << "Time taken: " << int(delay * 100000 + 0.5) << " micro seconds" << std::endl;
//	printf("frame : %d\nelapsed time : %d ms\nfps : %f\n", iter * 3, dwTotal, (float)(iter * 3) / dwTotal*1000);
	//printf("frame : %d\navg Elapsed time : %f ms\nfps : %f\n", iter, dwTotal/(float)iter, (float)(iter*3) / dwTotal * 1000);
		
}

void PipelineCheck()
{


	
	display();

	

	
	//printf("elapsed time : %d ms\n", dwEndTime - dwStartTime);
}
