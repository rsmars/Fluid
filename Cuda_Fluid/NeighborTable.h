#pragma once
#include "FluidSdx.h"
namespace SPH{
	class NeighborTable{
	public:
		/** reset neighbor table */
		void reset(unsigned int pointCounts);
		/** prepare a point neighbor data */
		void pointPrepare(unsigned int ptIndex);
		/** add neighbor data to current point */
		bool pointAddNeighbor(unsigned int ptIndex, float distance);
		/** commit point neighbor data to data buf*/
		void pointCommit(void);
		/** get point neighbor counts */
		int getNeighborCounts(unsigned int ptIndex) { return m_pointExtraData[ptIndex].neighborCounts; }
		/** get point neightbor information*/
		void getNeighborInfo(unsigned int ptIndex, int index, unsigned int& neighborIndex, float& neighborDistance);

	private:
		enum { MAX_NEIGHTBOR_COUNTS = 80, };

		union PointExtraData
		{
			struct
			{
				unsigned neighborDataOffset : 24;
				unsigned neighborCounts : 8;
			};

			unsigned int neighborData;
		};

		PointExtraData* m_pointExtraData;
		unsigned int m_pointCounts;
		unsigned int m_pointCapcity;

		unsigned char* m_neighborDataBuf;	//neighbor data buf
		unsigned int m_dataBufSize;			//in bytes
		unsigned int m_dataBufOffset;		//current neighbor data buf offset

		////// temp data for current point
		unsigned int m_currPoint;
		int m_currNeighborCounts;
		unsigned int m_currNeighborIndex[MAX_NEIGHTBOR_COUNTS];
		float m_currNeighborDistance[MAX_NEIGHTBOR_COUNTS];
	private:
		void _growDataBuf(unsigned int need_size);
	public:
		NeighborTable();
		~NeighborTable();
	};
}

