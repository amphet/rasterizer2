#include "Tessellator.h"


CTessellator::CTessellator()
{
	m_pFace = NULL;//dummy head�� �����ϴ°� ������

}


CTessellator::~CTessellator()
{
}

Triangular_Face* CTessellator::Launch(obj_face** original_face)
{
	/*
	objLoader�� �ִ� ������ ������ ������ tessellationó��
	Vertex Shader�� launch�� ���������� ��ȯ�� face���� m_pFace�� linked list�� �����ѵ� head�� ����


	�������⿡�� ������ �������̴ϱ� ó���ѹ��� tessellation�ϰ�, ���Ŀ��� �̹� ����� m_pFace�� �����ϸ� �ɰͰ���
	
	������� ������� m_bConverted ���� �����ڿ��� �ʱⰪ false�� �־��ش�����

	if(m_bConverted == false)
	{
		original_face������ ������ �ﰢ�� ���������� �ɰ���, m_pFace�� append

		m_bConverted = true;
	}

	return m_pFace;
	
	�̷�������


	original_face[i] �̷������� �����ϸ� �ɰ���.
	e.g. �׳� �ﰢ�� �������϶� ó���ϴ¹��
	
	Triangular_Face* temp2 = g_pTriangular_Face;
	for (int i = 0; i < objData->faceCount; i++)
	{
		obj_face *o = objData->faceList[i];
		temp2->next = new Triangular_Face;
		temp2 = temp2->next;
		temp2->a = o->vertex_index[0];
		temp2->b = o->vertex_index[1];
		temp2->c = o->vertex_index[2];
		temp2->next = NULL;
	
	}

	���� �����ÿ��� OBJLoader�� example.txt�����ؼ� ������ ������ ���� ó���� �����ϵ��� �ؾ� �ҵ�

	*/


	return m_pFace;
}
