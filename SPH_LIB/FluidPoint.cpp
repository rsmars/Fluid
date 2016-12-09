#include "FluidPoint.h"
namespace SPH{
	PointBuffer::PointBuffer() ://,
		MAX_COUNT(MAX_POINT),
		m_buf(nullptr){
		this->reset(1024);
	}
	PointBuffer::~PointBuffer(){
		if (m_buf){
			delete[] m_buf;
		}
	}
	bool PointBuffer::reset(unsigned int size){
		if (m_buf){
			delete[] m_buf;
			m_point_count = 0;
		}

		m_buf_capcity = size;
		m_buf = new Point[size];
		return true;
	}
	Point& PointBuffer::addPoint(){
		if (m_point_count >= m_buf_capcity){
			if (m_buf_capcity * 2 > MAX_COUNT){
				int idx = rand() % m_point_count;
				return m_buf[idx];
			}
			Point* nbuf = new Point[m_buf_capcity*2];
			memcpy(nbuf, m_buf, sizeof(Point)*m_buf_capcity);

			m_buf_capcity *= 2;
			delete[] m_buf;
			m_buf = nbuf;
		}
		return m_buf[m_point_count++];
	}
}
