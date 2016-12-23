#pragma once
#include "FluidSdx.h"
#include "FluidPoint.h"
#include <unordered_map>
#include <unordered_set>
namespace SPH{
	class GridContainer{
	public://method
		void init(const Box& box, float sim_scale, float cell_size, float border);
		void insertParticles(PointBuffer&);
		void insertParticlesSampling(PointBuffer&, unsigned int);
		int findCell(const float4& p);
		void findCell(const float4& p, float r, std::unordered_set<int>& res);
		int getGridCellIndex(float px, float py, float pz);
		int getGridCellIndex(float4 pos){
			return getGridCellIndex(pos.x, pos.y, pos.z);
		}
		int operator[](int idx){
			if (m_gridData.count(idx) == 0) return -1;
			return m_gridData[idx];
		}
	private: //data
		std::unordered_map<int, int> m_gridData;
		float4 m_gridMinPos, m_gridMaxPos;
		float4 m_gridSize;
		float4 m_gridDelta;
		int4 m_gridRes;
		float m_cellSize;
	public:
		GridContainer(){};
		~GridContainer(){};
	};
}