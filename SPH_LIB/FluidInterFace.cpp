//#include "FluidInterFace.h"
#include "FluidSystem.h"
SPH::System* getSPHSystem(void)
{
	static SPH::FluidSystem s_theSystem;
	return &s_theSystem;
}
//void mymath(int* a)
//{
//	*a = 1010;
//	return;
//}
void tick(void){
	getSPHSystem()->tick();
	getSPHSystem()->printPerformance();
}
void init(void){
	SPH::float4 wall_min = { -55, -55, -55 };
	SPH::float4 wall_max = { 55, 55, 55 };
	SPH::float4 fluid_min = { -35, 10, -35 };
	SPH::float4 fluid_max = { 35, 45, 35 };
	SPH::float4 gravity = { 0.0, -9.8f, 0 };
	//param
	const int maxPoints = 10000;
	getSPHSystem()->init(maxPoints, wall_min, wall_max, fluid_min, fluid_max, gravity);
}
const SPH::Point* getPoint(int idx){
	return getSPHSystem()->getPointBuf()+idx;
}
int getCount(void){
	return getSPHSystem()->getPointCounts();
}