#pragma once
#include "FluidSdx.h"
#include "FluidPoint.h"
#include "GridContainer.h"
#include "NeighborTable.h"
namespace SPH{

	class FluidSystem
	{
	public:
		virtual void init(unsigned int maxPointCounts,
			const float4 wallBox_min, const float4 wallBox_max,
			const float4 initFluidBox_min, const float4 initFluidBox_max,
			const float4 gravity)
		{
			_init(maxPointCounts,
				Box(float4(wallBox_min), float4(wallBox_max)),
				Box(float4(initFluidBox_min), float4(initFluidBox_max)),
				float4(gravity));
		}

		/** Get size of point in bytes */
		virtual unsigned int getPointStride(void) const { return sizeof(Point); }
		/** Get point counts */
		virtual unsigned int getPointCounts(void) const { return m_pointBuffer.size(); }
		/** Get Fluid Point Buffer */
		virtual const Point* getPointBuf(void) const { return (const Point*)&m_pointBuffer[0]; }
		/** 逻辑帧 */
		virtual void tick(void){
			m_gridContainer.insertParticles(m_pointBuffer);
			_resetNeighbor();
			_computePressure();
			_computeForce();
			_advance();
		}

	private:
		/** 初始化系统
		*/
		void _init(unsigned int maxPointCounts, const Box& wallBox, const Box& initFluidBox, const float4& gravity){
			m_pointBuffer.reset(maxPointCounts);
			m_sphWallBox = wallBox;
			m_gravityDir = gravity;
			// Create the particles
			float pointDistance = pow(m_pointMass / m_restDensity, 1.f / 3.f); //粒子间距
			_addFluidVolume(initFluidBox, pointDistance);

			// Setup grid Grid cell size (2r)	
			m_gridContainer.init(wallBox, m_smoothRadius*2.f, 1.0);
		}
		/** 计算相邻关系 */
		void _resetNeighbor(void){
			//h^2
			float h2 = m_smoothRadius*m_smoothRadius;

			m_neighborTable.reset(m_pointBuffer.size());
			unordered_set<int> cells;
			for (unsigned int i = 0; i < m_pointBuffer.size(); i++){
				Point& pi = m_pointBuffer[i];
				
				cells.clear();
				m_gridContainer.findCell(pi.position, m_smoothRadius, cells);
				m_neighborTable.pointPrepare(i);
				for (auto cidx : cells){
					int pidx = m_gridContainer[cidx];
					bool flag = true;
					while (pidx != -1 && flag){
						Point pj = m_pointBuffer[pidx];
						float4 line = pi.position - pj.position;
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
		/** 计算密度、压强 */
		void _computePressure(void){
			const float h2 = m_smoothRadius*m_smoothRadius;

			for (unsigned int i = 0; i < m_pointBuffer.size(); i++){
				unsigned int neighborCount = m_neighborTable.getNeighborCounts(i);
				Point& pi = m_pointBuffer[i];	
				float wsum = 0;
				for (unsigned int j = 0; j < neighborCount; j++){
					unsigned int jidx;
					float dis;
					m_neighborTable.getNeighborInfo(i, j, jidx, dis);
					float r2 = dis*dis;
					// + m_pointBuffer[jidx].mass*(h2-r2)^3;
					wsum = wsum + m_pointMass*pow((h2 - r2), 3.f);
				}

				//m_kernelPoly6 = 315.0f/(64.0f * 3.141592f * h^9);
				pi.density = m_kernelPoly6*wsum;
				pi.pressure = (pi.density - m_restDensity)*m_gasConstantK;
			}
		}
		/** 计算加速度 */
		void _computeForce(void){
			const float h2 = m_smoothRadius*m_smoothRadius;

			for (unsigned int i = 0; i < m_pointBuffer.size(); i++){
				Point& pi = m_pointBuffer[i];
				unsigned int neighborCount = m_neighborTable.getNeighborCounts(i);
				//float4 acc;
				float4 press;//_without Kernel Constant
				float4 vis;//_without Kernel Constant
				for (unsigned int j = 0; j < neighborCount; j++){
					unsigned int jidx;
					float dis, &r = dis;
					m_neighborTable.getNeighborInfo(i, j, jidx, dis);
					Point& pj = m_pointBuffer[jidx];
					//r(i)-r(j)
					float4 ri_rj = (pi.position - pj.position);
					//h-r
					float h_r = m_smoothRadius - r;
					//h^2-r^2
					float h2_r2 = h2 - r*r;
					//pressure
					float4 pterm = -ri_rj / r*(pi.pressure + pj.pressure) / (2.f * pi.density*pj.density)*h_r*h_r * m_pointMass;
					press = press + pterm;
					//viscosity
					float4 uj_ui = pj.velocity_eval - pj.velocity_eval;
					float4 vterm = uj_ui / (pi.density*pj.density) * h_r * m_pointMass * m_viscosity;
					vis = vis + vterm;
				}
				pi.acceleration = press + vis + m_gravityDir;
			}
		}
		/** 移动粒子*/
		void _advance(void){
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
				float diff = 2 * 1.f - (p.position.z - m_sphWallBox.minpos.z)*1.f;
				if (diff > 0.f)
				{
					float4 norm(0, 0, 1);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}

				diff = 2 * 1.f - (m_sphWallBox.maxpos.z - p.position.z)*1.f;
				if (diff > 0.f)
				{
					float4 norm(0, 0, -1);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}

				// X-axis walls
				diff = 2 * 1.f - (p.position.x - m_sphWallBox.minpos.x)*1.f;
				if (diff > 0.f)
				{
					float4 norm(1, 0, 0);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}

				diff = 2 * 1.f - (m_sphWallBox.maxpos.x - p.position.x)*1.f;
				if (diff > 0.f)
				{
					float4 norm(-1, 0, 0);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}

				// Y-axis walls
				diff = 2 * 1.f - (p.position.y - m_sphWallBox.minpos.y)*1.f;
				if (diff > 0.f)
				{
					float4 norm(0, 1, 0);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}
				diff = 2 * 1.f - (m_sphWallBox.maxpos.y - p.position.y)*1.f;
				if (diff > 0.f)
				{
					float4 norm(0, -1, 0);
					float adj = m_boundartStiffness * diff - m_boundaryDampening * norm.dot(p.velocity_eval);
					accel.x += adj * norm.x;
					accel.y += adj * norm.y;
					accel.z += adj * norm.z;
				}

				// Plane gravity
				//accel += m_gravityDir;

				// Leapfrog Integration ----------------------------
				float4 vnext = p.velocity + accel*deltaTime;			// v(t+1/2) = v(t-1/2) + a(t) dt			
				p.velocity_eval = (p.velocity + vnext)*0.5f;				// v(t+1) = [v(t-1/2) + v(t+1/2)] * 0.5		used to compute forces later
				p.velocity = vnext;
				p.position = p.position + vnext*deltaTime / 1.f;		// p(t+1) = p(t) + v(t+1/2) dt
			}
		}
		/** 创建初始液体块*/
		void _addFluidVolume(const Box& fluidBox, float spacing){
			auto pbegin = fluidBox.minpos;
			auto pend = fluidBox.maxpos;
			float4 pos;
			for (pos.z = pbegin.z ; pos.z <= pend.z; pos.z += spacing){
				for (pos.y = pbegin.y; pos.y <= pend.y; pos.y += spacing){
					for (pos.x = pbegin.x; pos.x <= pend.x; pos.x += spacing){
						Point& p = m_pointBuffer.addPoint();
						p.position = pos;
					}
				}
			}
		}

	private:
		PointBuffer m_pointBuffer;
		GridContainer m_gridContainer;
		NeighborTable m_neighborTable;

		// SPH Kernel
		float m_kernelPoly6;
		float m_kernelSpiky;
		float m_kernelViscosity;

		//Other Parameters
		//float 1.f;
		float m_viscosity;
		float m_restDensity;
		float m_pointMass;
		float m_smoothRadius;
		float m_gasConstantK;
		float m_boundartStiffness;
		float m_boundaryDampening;
		float m_speedLimiting;
		float4 m_gravityDir;

		Box m_sphWallBox;
	public:
		FluidSystem(){
			//1.f = 0.004f;			// 尺寸单位
			m_viscosity = 1.0f;				// 粘度
			m_restDensity = 1000.f;			// 密度
			m_pointMass = 0.0004f;			// 粒子质量
			m_gasConstantK = 1.0f;			// 理想气体方程常量
			m_smoothRadius = 0.01f;			// 光滑核半径

			m_boundartStiffness = 10000.f;
			m_boundaryDampening = 256.f;
			m_speedLimiting = 200.f;

			//Poly6 Kernel
			m_kernelPoly6 = 315.0f / (64.0f * 3.141592f * pow(m_smoothRadius, 9));
			//Spiky Kernel
			m_kernelSpiky = -45.0f / (3.141592f * pow(m_smoothRadius, 6));
			//Viscosity Kernel
			m_kernelViscosity = 45.0f / (3.141592f * pow(m_smoothRadius, 6));
		}

		~FluidSystem(){

		}
	};
}