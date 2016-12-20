#include "FluidSystem.h"

namespace SPH{
	void FluidSystem::_init(unsigned int maxPointCounts, const Box& wallBox, const Box& initFluidBox, const float4& gravity){
		m_pointBuffer.reset(maxPointCounts);
		m_sphWallBox = wallBox;
		m_gravityDir = gravity;
		// Create the particles
		float pointDistance = pow(m_pointMass / m_restDensity, 1.f / 3.f); //аёвс╪Д╬Ю
		_addFluidVolume(initFluidBox, pointDistance / m_unitScale);

		// Setup grid Grid cell size (2r)	
		m_gridContainer.init(wallBox, m_unitScale, m_smoothRadius*2.f, 1.0);
	}
	void FluidSystem::_resetNeighbor(void){
		//h^2
		float h2 = m_smoothRadius*m_smoothRadius;

		m_neighborTable.reset(m_pointBuffer.size());
		unordered_set<int> cells;
		for (unsigned int i = 0; i < m_pointBuffer.size(); i++){
			Point& pi = m_pointBuffer[i];

			cells.clear();
			m_gridContainer.findCell(pi.position, m_smoothRadius / m_unitScale, cells);
			m_neighborTable.pointPrepare(i);
			for (auto cidx : cells){
				int pidx = m_gridContainer[cidx];
				bool flag = true;
				while (pidx != -1 && flag){
					Point pj = m_pointBuffer[pidx];
					float4 line = (pi.position - pj.position)*m_unitScale;
					float r2 = line.dot(line);
					if (r2 <= h2){
						flag = flag && m_neighborTable.pointAddNeighbor(pidx, sqrt(r2));
					}
					pidx = pj.pnext;
				}
				if (!flag)
					break;
			}
			m_neighborTable.pointCommit();

		}
	}

	void FluidSystem::_computePressure(void){
		clock_t  start = 0, stop;
		start = clock();
		//h^2
		float h2 = m_smoothRadius*m_smoothRadius;

		//reset neightbor table
		m_neighborTable.reset(m_pointBuffer.size());

		for (unsigned int i = 0; i<m_pointBuffer.size(); i++)
		{
			Point& pi = m_pointBuffer[i];

			float sum = 0.f;
			m_neighborTable.pointPrepare(i);

			unordered_set<int> gridCell;
			m_gridContainer.findCell(pi.position, m_smoothRadius / m_unitScale, gridCell);

			for (int cidx : gridCell)
			{
				int pndx = m_gridContainer[cidx];
				while (pndx != -1)
				{
					Point& pj = m_pointBuffer[pndx];
					if (pndx == i)
					{
						sum += pow(h2, 3.f);  //self
					}
					else
					{
						float4 pi_pj = (pi.position - pj.position)*m_unitScale;
						float r2 = pi_pj.dot(pi_pj);
						if (h2 > r2)
						{
							float h2_r2 = h2 - r2;
							sum += pow(h2_r2, 3.f);  //(h^2-r^2)^3

							if (!m_neighborTable.pointAddNeighbor(pndx, sqrt(r2)))
							{
								goto NEIGHBOR_FULL;
							}
						}
					}
					pndx = pj.pnext;
				}

			}

		NEIGHBOR_FULL:
			m_neighborTable.pointCommit();

			//m_kernelPoly6 = 315.0f/(64.0f * 3.141592f * h^9);
			pi.density = m_kernelPoly6*m_pointMass*sum;
			pi.pressure = (pi.density - m_restDensity)*m_gasConstantK;
		}
		stop = clock();
		pressCost = 1.0f*(stop - start) / CLOCKS_PER_SEC;
	}

	void FluidSystem::_computeForce(void){
		clock_t  start = 0, stop;
		start = clock();
		float h2 = m_smoothRadius*m_smoothRadius;

		for (unsigned int i = 0; i<m_pointBuffer.size(); i++)
		{
			Point& pi = m_pointBuffer[i];

			float4 accel_sum = 0.f;
			int neighborCounts = m_neighborTable.getNeighborCounts(i);

			for (int j = 0; j <neighborCounts; j++)
			{
				unsigned int neighborIndex;
				float r;
				m_neighborTable.getNeighborInfo(i, j, neighborIndex, r);

				Point& pj = m_pointBuffer[neighborIndex];
				//r(i)-r(j)
				float4 ri_rj = (pi.position - pj.position)*m_unitScale;
				//h-r
				float h_r = m_smoothRadius - r;
				//h^2-r^2
				float h2_r2 = h2 - r*r;

				//F_Pressure
				//m_kernelSpiky = -45.0f/(3.141592f * h^6);			
				float pterm = -m_pointMass*m_kernelSpiky*h_r*h_r*(pi.pressure + pj.pressure) / (2.f * pi.density * pj.density);
				accel_sum = accel_sum + ri_rj*pterm / r;

				//F_Viscosity
				//m_kernelViscosity = 45.0f/(3.141592f * h^6);
				float vterm = m_kernelViscosity * m_viscosity * h_r * m_pointMass / (pi.density * pj.density);
				accel_sum = accel_sum + (pj.velocity_eval - pi.velocity_eval)*vterm;
			}

			pi.acceleration = accel_sum;
		}
		stop = clock();
		forceCost = 1.0f*(stop - start) / CLOCKS_PER_SEC;
	}

	void FluidSystem::_advance(void){
		clock_t  start = 0, stop;
		start = clock();
		//fixed delta time per frame
		float deltaTime = 0.003f;

		float SL2 = m_speedLimiting*m_speedLimiting;

		for (unsigned int i = 0; i<m_pointBuffer.size(); i++)
		{
			Point& p = m_pointBuffer[i];

			// Compute Acceleration		
			float4 accel = p.acceleration;

			// Velocity limiting 
			float accel_2 = accel.dot(accel);
			if (accel_2 > SL2)
			{
				accel = accel * m_speedLimiting / sqrt(accel_2);
			}

			// Boundary Conditions

			// Z-axis walls
			float diff = 2 * m_unitScale - (p.position.z - m_sphWallBox.minpos.z)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(0, 0, 1);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}

			diff = 2 * m_unitScale - (m_sphWallBox.maxpos.z - p.position.z)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(0, 0, -1);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}

			// X-axis walls
			diff = 2 * m_unitScale - (p.position.x - m_sphWallBox.minpos.x)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(1, 0, 0);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}

			diff = 2 * m_unitScale - (m_sphWallBox.maxpos.x - p.position.x)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(-1, 0, 0);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}

			// Y-axis walls
			diff = 2 * m_unitScale - (p.position.y - m_sphWallBox.minpos.y)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(0, 1, 0);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}
			diff = 2 * m_unitScale - (m_sphWallBox.maxpos.y - p.position.y)*m_unitScale;
			if (diff > 0.f)
			{
				float4 norm(0, -1, 0);
				float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
				accel.x += adj * norm.x;
				accel.y += adj * norm.y;
				accel.z += adj * norm.z;
			}

			// Plane gravity
			accel = accel + m_gravityDir;

			// Leapfrog Integration ----------------------------
			float4 vnext = p.velocity + accel*deltaTime;			// v(t+1/2) = v(t-1/2) + a(t) dt			
			p.velocity_eval = (p.velocity + vnext)*0.5f;				// v(t+1) = [v(t-1/2) + v(t+1/2)] * 0.5		used to compute forces later
			p.velocity = vnext;
			p.position = p.position + vnext*deltaTime / m_unitScale;		// p(t+1) = p(t) + v(t+1/2) dt
		}

		stop = clock();
		advCost = 1.0f*(stop - start) / CLOCKS_PER_SEC;
	}
}

