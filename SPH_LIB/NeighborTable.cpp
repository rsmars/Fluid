#include"NeighborTable.h"
namespace SPH{
	NeighborTable::NeighborTable()
		: m_pointExtraData(0)
		, m_pointCounts(0)
		, m_pointCapcity(0)
		, m_neighborDataBuf(0)
		, m_dataBufSize(0)
		, m_currNeighborCounts(0)
		, m_currPoint(0)
		, m_dataBufOffset(0)
	{

	}
	NeighborTable::~NeighborTable(){
		if (m_pointExtraData) delete[] m_pointExtraData;
		if (m_neighborDataBuf) delete[] m_neighborDataBuf;
	}
	void NeighborTable::reset(unsigned int pointCounts){
		if (pointCounts>m_pointCapcity)
		{
			if (m_pointExtraData)
			{
				free(m_pointExtraData);
			}
			m_pointExtraData = new PointExtraData[pointCounts];
			m_pointCapcity = pointCounts;
		}

		m_pointCounts = pointCounts;
		memset(m_pointExtraData, 0, sizeof(PointExtraData)*m_pointCapcity);
		m_dataBufOffset = 0;
	}
	void NeighborTable::_growDataBuf(unsigned int need_size){
		if (need_size < m_dataBufSize) return;
		unsigned int new_size = std::max(1024u, m_dataBufSize);
		while (new_size < need_size) new_size *= 2;
		unsigned char* newBuff = new unsigned char[new_size];
		memcpy(newBuff, m_neighborDataBuf, m_dataBufSize);
		delete[] m_neighborDataBuf;
		m_neighborDataBuf = newBuff;
		m_dataBufSize = new_size;
	}
	void NeighborTable::pointPrepare(unsigned int ptIndex){
		m_currPoint = ptIndex;
		m_currNeighborCounts = 0;
	}
	bool NeighborTable::pointAddNeighbor(unsigned int ptIndex, float distance){
		if (m_currNeighborCounts >= MAX_NEIGHTBOR_COUNTS) return false;
		int currIndex = m_currNeighborCounts++;
		m_currNeighborIndex[currIndex] = ptIndex;
		m_currNeighborDistance[currIndex] = distance;
		return true;
	}
	void NeighborTable::pointCommit(void){
		if (m_currNeighborCounts == 0) return;
		//calc the size needed;
		int dis_size = m_currNeighborCounts * sizeof(float);
		int idx_size = m_currNeighborCounts * sizeof(unsigned int);
		int need_size = m_dataBufOffset + dis_size + idx_size;
		_growDataBuf(need_size);
		//set neigbor data
		m_pointExtraData[m_currPoint].neighborCounts = m_currNeighborCounts;
		m_pointExtraData[m_currPoint].neighborDataOffset = m_dataBufOffset;
		//copy idx;
		memcpy(m_neighborDataBuf + m_dataBufOffset, m_currNeighborIndex, idx_size);
		m_dataBufOffset += idx_size;
		//cp dis
		memcpy(m_neighborDataBuf + m_dataBufOffset, m_currNeighborDistance, dis_size);
		m_dataBufOffset += dis_size;
	}
	void NeighborTable::getNeighborInfo(unsigned int ptIndex, int index, unsigned int& neighborIndex, float& neighborDistance){
		const PointExtraData neighData = m_pointExtraData[ptIndex];
		unsigned int* indexBuf = (unsigned int*)(m_neighborDataBuf + neighData.neighborDataOffset);
		float* distanceBuf = (float*)(m_neighborDataBuf + neighData.neighborDataOffset + sizeof(unsigned int)*neighData.neighborCounts);
		neighborIndex = indexBuf[index];
		neighborDistance = distanceBuf[index];
	}

}
