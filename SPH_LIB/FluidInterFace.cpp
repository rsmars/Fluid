//#include "FluidInterFace.h"
#include "FluidSystem.h"
#include "GridInterFluidSystem.h"
SPH::System* getGridInterSPHSystem(void)
{
	static SPH::GridInterFluidSystem s_theSystem;
	return &s_theSystem;
}
SPH::System* getSPHSystem(void)
{
	static SPH::FluidSystem s_theSystem;
	return &s_theSystem;
}
