#pragma once
#include"FluidSdx.h"


namespace SPH{
	struct Point{
		float pressure;
		float density;
		float4 velocity;
		float4 velocity_eval;
		float4 acceleration;
		float4 position;

		int pnext;
	};
	class PointBuffer{
	public:
		const int MAX_COUNT;
	public://method
		bool reset(unsigned int size);
		unsigned int size() const{ return m_point_count; };
		Point& operator[](int idx){ return m_buf[idx]; };
		const Point& operator[](int idx) const { return m_buf[idx]; };
		Point& addPoint();
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
