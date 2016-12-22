#pragma once
#include"FluidSdx.h"
#include "FluidInterFace.h"

namespace SPH{
	
	class PointBuffer{
	public:
		const int MAX_COUNT;
	public://method
		bool reset(unsigned int size);
		unsigned int size() const{ return m_point_count; };
		Point& operator[](int idx){ return m_buf[idx]; };
		const Point& operator[](int idx) const { return m_buf[idx]; };
		Point& addPoint();
		void pbSwap(Point &a, Point &b) {
			Point tmp = a;
			a = b;
			b = tmp;
		}
		void shuffle(int cnt);
	private://data
		//point data buffer
		Point* m_buf;
		int m_buf_capcity;
		int m_point_count;
	public:
		PointBuffer();
		~PointBuffer();
	};
}
