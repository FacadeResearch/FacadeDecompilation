#pragma once
#include "Controller.h"

class Sprite {
	public:
		Sprite();
		Controller g_Controller;
		int m_NumSurfaces;
		int m_Id;
		int m_ScaleX, m_ScaleY;
		float m_BoundingBox[12];
		char* m_SurfaceData;
		const char* m_Name;
		bool m_Loop;
		int m_Speed;
		int m_startFrame; //??
};