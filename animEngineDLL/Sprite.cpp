#include "Sprite.h"
#include <cstring>

Sprite::Sprite()
{
	g_Controller = Controller();
	m_NumSurfaces = 0;
	m_Id = 0;
	m_ScaleX = 0;
	m_ScaleY = 0;
	m_SurfaceData = nullptr;
	m_Name = "";
	m_Loop = false;
	m_Speed = 0;
	m_startFrame = -1; //??
	
	memset(m_BoundingBox, 0, sizeof(m_BoundingBox));
}