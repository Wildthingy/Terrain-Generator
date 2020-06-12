#include "Terrain.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Terrain::Terrain(int width, int height, int iterations, int method, float snow)
{
	m_TerrainLength = width;
	m_TerrainWidth = height;
	generationMethod = method;
	m_generationIterations = iterations;
	snowHeight = snow;

	GenerateTerrain();

	BuildTerrainBuffer();

	m_pTerrainBuffer = CreateImmutableVertexBuffer(Application::s_pApp->GetDevice(), sizeof Vertex_Pos3fColour4ubNormal3f * m_TerrainVtxCount, m_pMapVtxs);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Terrain::~Terrain()
{
	Release(m_pTerrainBuffer);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void Terrain::Draw(void)
{
	Application::s_pApp->DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, m_pTerrainBuffer, NULL, m_TerrainVtxCount);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool Terrain::GenerateTerrain()
{
	int i, j, index;

	// Create the structure to hold the height map data.
	m_pTerrain = new XMFLOAT3[m_TerrainWidth * m_TerrainLength];

	if(!m_pTerrain)
	{
		return false;
	}

	for(j = 0; j < m_TerrainLength; j++)
	{
		for(i = 0; i < m_TerrainWidth; i++)
		{
			index = (m_TerrainWidth * j) + i;

			m_pTerrain[index].x = (float)(i - (m_TerrainWidth  / 2));
			m_pTerrain[index].y = 0.f;
			m_pTerrain[index].z = (float)(j - (m_TerrainLength / 2));
		}
	}

	srand(time(NULL));
	switch (generationMethod)
	{
	case 0:
		FaultAlgorithm();
		break;
	case 1:
		CirclesAlgorithm();
		break;
	case 2:
		ParticleDepositionAlgorithm();
		break;
	default:
		break;
	}

	SmoothTerrain();

	return true;
}

bool Terrain::SmoothTerrain()
{
	int i, j, index;

	for (j = 0; j < m_TerrainLength - 1; j++)
	{
		for (i = 0; i < m_TerrainWidth - 1; i++)
		{
			index = (m_TerrainWidth * j) + i;

			m_pTerrain[index].y = (m_pTerrain[index].y + m_pTerrain[index + 1].y + m_pTerrain[index + m_TerrainWidth].y) / 3;
		}
	}

	for (j = m_TerrainLength - 1; j > 0; j--)
	{
		for (i = m_TerrainWidth - 1; i > 0; i--)
		{
			index = (m_TerrainWidth * j) + i;

			m_pTerrain[index].y = (m_pTerrain[index].y + m_pTerrain[index - 1].y + m_pTerrain[index - m_TerrainWidth].y) / 3;
		}
	}
	return true;
}

bool Terrain::BuildTerrainBuffer()
{
	m_pTerrainBuffer = NULL;

	static const VertexColour GRASS_COLOUR( 76, 153,   0, 255);
	static const VertexColour  ROCK_COLOUR( 96,  96,  96, 255);
	static const VertexColour  SNOW_COLOUR(235, 235, 235, 255);

	m_TerrainVtxCount = (m_TerrainLength - 1) * (m_TerrainWidth - 1) * 6;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3f[m_TerrainVtxCount];

	int vtxIndex = 0;
	int mapIndex = 0;

	XMVECTOR v0, v1, v2, v3;
	VertexColour  c0, c1, c2, c3, c4, c5;
	XMVECTOR vertical = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	for (int l = 0; l < m_TerrainLength; ++l)
	{
		for (int w = 0; w < m_TerrainWidth; ++w)
		{
			if (w < m_TerrainWidth - 1 && l < m_TerrainLength - 1)
			{
				v0 = XMLoadFloat3(&m_pTerrain[mapIndex]);
				v1 = XMLoadFloat3(&m_pTerrain[mapIndex + m_TerrainWidth]);
				v2 = XMLoadFloat3(&m_pTerrain[mapIndex + 1]);
				v3 = XMLoadFloat3(&m_pTerrain[mapIndex + m_TerrainWidth + 1]);

				if (m_pTerrain[mapIndex].y > snowHeight * m_generationIterations)
					c0 = SNOW_COLOUR;
				else
					c0 = GRASS_COLOUR;
				if (m_pTerrain[mapIndex + m_TerrainWidth].y > snowHeight * m_generationIterations)
					c1 = SNOW_COLOUR;
				else
					c1 = GRASS_COLOUR;
				if (m_pTerrain[mapIndex + 1].y > snowHeight * m_generationIterations)
					c2 = SNOW_COLOUR;
				else
					c2 = GRASS_COLOUR;
				if (m_pTerrain[mapIndex + 1].y > snowHeight * m_generationIterations)
					c3 = SNOW_COLOUR;
				else
					c3 = GRASS_COLOUR;
				if (m_pTerrain[mapIndex + m_TerrainWidth].y > snowHeight * m_generationIterations)
					c4 = SNOW_COLOUR;
				else
					c4 = GRASS_COLOUR;
				if (m_pTerrain[mapIndex + m_TerrainWidth + 1].y > snowHeight * m_generationIterations)
					c5 = SNOW_COLOUR;
				else
					c5 = GRASS_COLOUR;

				XMVECTOR vA = v0 - v1;
				XMVECTOR vB = v1 - v2;
				XMVECTOR vC = v3 - v1;

				XMVECTOR vN1, vN2;
				XMFLOAT3 fD;
				vN1 = XMVector3Cross(vA, vB);
				vN1 = XMVector3Normalize(vN1);


				XMStoreFloat3(&fD, XMVector3AngleBetweenVectors(vertical, vN1));

				if (fD.x > .5)
				{
					c0 = ROCK_COLOUR;
					c1 = ROCK_COLOUR;
					c2 = ROCK_COLOUR;
				}

				vN2 = XMVector3Cross(vB, vC);
				vN2 = XMVector3Normalize(vN2);

				XMStoreFloat3(&fD, XMVector3AngleBetweenVectors(vertical, vN2));

				if (fD.x > .5)
				{
					c3 = ROCK_COLOUR;
					c4 = ROCK_COLOUR;
					c5 = ROCK_COLOUR;
				}

				m_pMapVtxs[vtxIndex + 0] = Vertex_Pos3fColour4ubNormal3f(v0, c0, vN1);
				m_pMapVtxs[vtxIndex + 1] = Vertex_Pos3fColour4ubNormal3f(v1, c1, vN1);
				m_pMapVtxs[vtxIndex + 2] = Vertex_Pos3fColour4ubNormal3f(v2, c2, vN1);
				m_pMapVtxs[vtxIndex + 3] = Vertex_Pos3fColour4ubNormal3f(v2, c3, vN2);
				m_pMapVtxs[vtxIndex + 4] = Vertex_Pos3fColour4ubNormal3f(v1, c4, vN2);
				m_pMapVtxs[vtxIndex + 5] = Vertex_Pos3fColour4ubNormal3f(v3, c5, vN2);

				vtxIndex += 6;
			}
			mapIndex++;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Implementation of "Fault" terrain generation algorithm
//////////////////////////////////////////////////////////////////////
void Terrain::FaultAlgorithm()
{
	int r1, r2;
	float a, b, c;

	for (int k = 0; k < m_generationIterations; k++)
	{
		r1 = rand() % (m_TerrainWidth * m_TerrainLength);
		r2 = rand() % (m_TerrainWidth * m_TerrainLength);

		a = m_pTerrain[r2].z - m_pTerrain[r1].z;
		b = - m_pTerrain[r2].x - m_pTerrain[r1].x;
		c = -(m_pTerrain[r1].x * a) + (m_pTerrain[r1].z * b);

		for (int j = 0; j < m_TerrainWidth * m_TerrainLength; j++)
		{
			if (a * m_pTerrain[j].x + b * m_pTerrain[j].z - c > 0)
				m_pTerrain[j].y += 1;
			else
				m_pTerrain[j].y -= 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Implementation of "Circles" terrain generation algorithm
//////////////////////////////////////////////////////////////////////
void Terrain::CirclesAlgorithm()
{
	int r, a;

	for (int k = 0; k < m_generationIterations; k++)
	{
		r = rand() % (m_TerrainWidth / 2);

		a = rand() % (m_TerrainWidth * m_TerrainLength);

		for (int j = 0; j < m_TerrainWidth * m_TerrainLength; j++)
		{
			if (((m_pTerrain[j].x - m_pTerrain[a].x) * (m_pTerrain[j].x - m_pTerrain[a].x))
			   +((m_pTerrain[j].z - m_pTerrain[a].z) * (m_pTerrain[j].z - m_pTerrain[a].z))
			   -(r * r) < 0)
				m_pTerrain[j].y += 1;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Implementation of "Particle Deposition" terrain generation algorithm
//////////////////////////////////////////////////////////////////////
void Terrain::ParticleDepositionAlgorithm()
{
	int point = rand() % (m_TerrainWidth * m_TerrainLength);

	for (int k = 0; k < m_generationIterations; k++)
	{
		m_pTerrain[point].y += 1;

		switch (rand() % 25) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5: // move right
			if (point < m_TerrainWidth * m_TerrainLength - 1)
				point++; 
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11: // move left
			if (point > 0)
				point--;
			break;
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17: // move closer
			if (point <= m_TerrainWidth * (m_TerrainLength - 1) - 1)
				point += m_TerrainWidth;
			break;
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23: // move away
			if (point >= m_TerrainWidth)
				point -= m_TerrainWidth;
			break;
		case 24:
			point = rand() % (m_TerrainWidth * m_TerrainLength);
			break;
		}
	}
}