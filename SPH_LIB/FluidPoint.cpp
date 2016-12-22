#include "FluidPoint.h"
#include <random>
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
	void PointBuffer::shuffle(int cnt) {
		std::random_device rd;
		std::mt19937_64 g(rd());

		typedef typename std::uniform_int_distribution<int> udistr_int;
		typedef typename udistr_int::param_type udistr_param;

		udistr_int D;
		for (int i = m_point_count - 1; i > m_point_count - 1 - cnt; --i) {
			pbSwap(m_buf[i], m_buf[D(g, udistr_param(0, i))]);
		}
	}
}
