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

};
