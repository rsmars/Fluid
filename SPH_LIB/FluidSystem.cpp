#include "FluidSystem.h"

SPH::System* getSPHSystem(void)
{
	static SPH::FluidSystem s_theSystem;
	return &s_theSystem;
}
