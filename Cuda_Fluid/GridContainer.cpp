#include "GridContainer.h"

namespace SPH{
	void GridContainer::init(const Box& box, float cell_size, float border){
		m_gridMinPos = box.minpos; m_gridMinPos = m_gridMinPos - border;
		m_gridMaxPos = box.maxpos; m_gridMaxPos = m_gridMaxPos + border;
		m_gridSize = m_gridMaxPos; m_gridSize = m_gridSize - m_gridMinPos;
		m_gridRes.x = (int)ceil(m_gridSize.x / cell_size);
		m_gridRes.y = (int)ceil(m_gridSize.y / cell_size);
		m_gridRes.z = (int)ceil(m_gridSize.z / cell_size);
		m_gridSize.x = m_gridRes.x * cell_size;
		m_gridSize.y = m_gridRes.y * cell_size;
		m_gridSize.z = m_gridRes.z * cell_size;
		// cellPos = worldPos .* gridDelta
		m_gridDelta = m_gridRes;//
		m_gridDelta = m_gridDelta / m_gridSize;
		m_gridData.clear();
		m_cellSize = cell_size;
		//m_gridData = std::unordered_map<int, int>(-1);
	}
	int GridContainer::getGridCellIndex(float px, float py, float pz)
	{
		int gx = (int)((px - m_gridMinPos.x) * m_gridDelta.x);
		int gy = (int)((py - m_gridMinPos.y) * m_gridDelta.y);
		int gz = (int)((pz - m_gridMinPos.z) * m_gridDelta.z);
		return (gz*m_gridRes.y + gy)*m_gridRes.x + gx;
	}

	void GridContainer::insertParticles(PointBuffer& buf){
		for (unsigned int i = 0; i < buf.size(); i++){
			Point& p = buf[i];
			int cidx = getGridCellIndex(p.position);
			if (m_gridData.count(cidx) > 0){
				p.pnext = m_gridData[cidx];
			}
			else p.pnext = -1;
			m_gridData[cidx] = i;
		}
	}


	int GridContainer::findCell(const float4& p){
		int idx = getGridCellIndex(p);
		if (idx < 0 || m_gridData.count(idx) == 0) return -1;
		return idx;
	}
	void GridContainer::findCell(const float4& p, float radius, std::unordered_set<int>& res){
		//int idx = findCell(p);
		float r = 0;
		while (r < radius + m_cellSize){
			for (float theta = 0; theta < PI + 1e-5; theta = theta + PI / 4){
				for (float phi = 0; phi < 2 * PI + 1e-5; phi = phi + PI / 4){
					float4 pos;
					pos.x = r * sin(theta) * cos(phi);
					pos.y = r * sin(theta) * sin(phi);
					pos.z = r * cos(theta);
					pos = pos + p;
					int idx = findCell(pos);
					if (idx != -1) res.insert(idx);
				}
			}
			r += m_cellSize;
		}
		
	}
}