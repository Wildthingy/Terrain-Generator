#ifndef Terrain_H
#define Terrain_H

#include "Application.h"
#include <time.h>


class Terrain
{
  public:
	Terrain(int, int, int, int, float);
	~Terrain();

	void Draw(void);


  private:
	bool GenerateTerrain();
	bool SmoothTerrain();
	bool BuildTerrainBuffer();

	void FaultAlgorithm();
	void CirclesAlgorithm();
	void ParticleDepositionAlgorithm();

	ID3D11Buffer* m_pTerrainBuffer;

	int m_TerrainWidth;
	int m_TerrainLength;
	int m_TerrainVtxCount;
	int m_TerrainFaceCount;
	int m_FacesPerRow;

	int m_generationIterations;
	int generationMethod;
	float snowHeight;

	XMFLOAT3* m_pTerrain;
	Vertex_Pos3fColour4ubNormal3f* m_pMapVtxs;
};

#endif