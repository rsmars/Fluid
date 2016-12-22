#pragma once
#include "FluidMath.h"
namespace SPH
{
	struct Point{
		float pressure;
		float density;
		float4 velocity;
		float4 velocity_eval;
		float4 acceleration;
		float4 position;

		int pnext;
		Point(){
			pressure = 0;
			density = 0;
			velocity = 0;
			velocity_eval = 0;
			acceleration = 0;
			position = 0;
		}
		Point& operator=(const Point &other) {
			if (this != &other) {
				this->pressure = other.pressure;
				this->density = other.density;
				this->velocity = other.velocity;
				this->velocity_eval = other.velocity_eval;
				this->acceleration = other.acceleration;
				this->position = other.position;
			}
			return *this;
		}
	};
	class System
	{
	public:
		virtual void init(unsigned int maxPointCounts,
			const float4 wallBox_min, const float4 wallBox_max,
			const float4 initFluidBox_min, const float4 initFluidBox_max,
			const float4 gravity) = 0;

		virtual unsigned int getPointStride(void) const = 0;
		virtual unsigned int getPointCounts(void) const = 0;
		virtual const Point* getPointBuf(void) const = 0;
		virtual void tick(void) = 0;
		virtual void printPerformance(void) = 0;
	};

}

extern "C"
{

	/** Get the sigleton SPH System point
	*/
	__declspec(dllexport) SPH::System* getSPHSystem(void);
	__declspec(dllexport) void tick(void);
	__declspec(dllexport) void init(void);
	__declspec(dllexport) const SPH::Point* getPoint(int idx);
	__declspec(dllexport) int getCount(void);

	//_declspec(dllexport) void mymath(int* a);
};