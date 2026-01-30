#pragma once
#include "Controller.h"

struct ModelPoint {
	float x;
	float y;
	float z;
};

struct ModelInfo {
	float radius;
	float fillColorIdx;
	float outlineStart;
	float outlineEnd;
	float outlineColorIdx;
	float outlineWidth;
	float flags;
};

struct ModelQuadLine {
	int p0;
	int p1;
	int ctrl;
	int type;
	int circleColor;
	int lineColor;
	int lineWidthA;
	int lineWidthB;
};

struct ModelQuadPoly {
	int p0;
	int p1;
	int p2;
	int p3;
	int type;
	int fillColor;
	int edgeColor0;
	int edgeColor1;
	int edgeColor2;
	int edgeColor3;
	int edgeWidth0;
	int edgeWidth1;
	int edgeWidth2;
	int edgeWidth3;
	int texFrame;
};

extern ModelPoint g_Couch_modelFrame[];
extern ModelInfo g_Couch_modelInfo[];
extern ModelQuadLine g_Couch_modelQuadLines[];
extern ModelQuadPoly g_Couch_modelQuadPolys[];

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
