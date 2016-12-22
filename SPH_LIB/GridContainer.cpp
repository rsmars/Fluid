#include "GridContainer.h"

namespace SPH{
	void GridContainer::init(const Box& box, float sim_scale, float cell_size, float border){
		float worldcell_size = cell_size / sim_scale;
		m_gridMinPos = box.minpos; m_gridMinPos = m_gridMinPos - border;
		m_gridMaxPos = box.maxpos; m_gridMaxPos = m_gridMaxPos + border;
		m_gridSize = m_gridMaxPos; m_gridSize = m_gridSize - m_gridMinPos;
		m_gridRes.x = (int)ceil(m_gridSize.x / worldcell_size);
		m_gridRes.y = (int)ceil(m_gridSize.y / worldcell_size);
		m_gridRes.z = (int)ceil(m_gridSize.z / worldcell_size);
		m_gridSize.x = m_gridRes.x * worldcell_size;
		m_gridSize.y = m_gridRes.y * worldcell_size;
		m_gridSize.z = m_gridRes.z * worldcell_size;
		// cellPos = worldPos .* gridDelta
		m_gridDelta = m_gridRes;//
		m_gridDelta = m_gridDelta / m_gridSize;
		m_gridData.clear();
		m_cellSize = worldcell_size;
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
		m_gridData.clear();
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

	void GridContainer::insertParticlesRandomSampling(PointBuffer& buf, float percentage) {
		assert(percentage > 0 && percentage < 1);
		m_gridData.clear();
		unsigned int sampleSize = buf.size()*percentage;
		buf.shuffle(sampleSize);
		unsigned int bufSize = buf.size();
		for (unsigned int i = bufSize-1; i > bufSize-1-sampleSize; --i) {
			Point &p = buf[i];
			int cidx = getGridCellIndex(p.position);
			if (m_gridData.count(cidx) > 0) {
				p.pnext = m_gridData[cidx];
			}
			else {
				p.pnext = -1;
			}
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
		/*float r = 0;
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
		return;*/
		// Compute sphere range
		int sph_min_x = (int)((-radius + p.x - m_gridMinPos.x) * m_gridDelta.x);
		int sph_min_y = (int)((-radius + p.y - m_gridMinPos.y) * m_gridDelta.y);
		int sph_min_z = (int)((-radius + p.z - m_gridMinPos.z) * m_gridDelta.z);
		if (sph_min_x < 0) sph_min_x = 0;
		if (sph_min_y < 0) sph_min_y = 0;
		if (sph_min_z < 0) sph_min_z = 0;
		int gridCell = (sph_min_z*m_gridRes.y + sph_min_y)*m_gridRes.x + sph_min_x;
		res.insert(gridCell);
		res.insert(gridCell + 1);
		res.insert(gridCell + m_gridRes.x);
		res.insert(gridCell + m_gridRes.x + 1);

		if (sph_min_z + 1 < m_gridRes.z) {
			res.insert((int)(gridCell + m_gridRes.y*m_gridRes.x));
			res.insert((int)(gridCell + m_gridRes.y*m_gridRes.x + 1));
			res.insert((int)(gridCell + m_gridRes.y*m_gridRes.x + m_gridRes.x));
			res.insert((int)(gridCell + m_gridRes.y*m_gridRes.x + m_gridRes.x + 1));
		}		
	}
}