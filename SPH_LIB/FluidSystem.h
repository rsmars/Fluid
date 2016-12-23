#pragma once
#include "FluidSdx.h"
#include "FluidPoint.h"
#include "GridContainer.h"
#include "NeighborTable.h"
#include "time.h"
#include "FluidInterFace.h"

namespace SPH{

	class FluidSystem : public System
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
		int cnt = 0;
		virtual void tick(void){
			clock_t  start = 0, stop;
			start = clock();

			float percentage = 1.f;
			unsigned int sampleSize = (unsigned int)(m_pointBuffer.size()*percentage);
			m_pointBuffer.shuffle(sampleSize);
			//m_gridContainer.insertParticles(m_pointBuffer);
			// 1. pointBuffer random select X%
			m_gridContainer.insertParticlesSampling(m_pointBuffer, sampleSize);
			//_resetNeighbor();		
			_computePressure();
			_computeForce();
			_advance();
			stop = clock();
			frameCost = 1.0f*(stop - start) / CLOCKS_PER_SEC;
		}
		void printPerformance(){
			printf("--------%d Particles(Frame %d)---------\n", this->getPointCounts(), ++cnt);
			printf("Time  Cost: %.4fs(%.4f fps)\n", frameCost, 1.f / frameCost);
			printf("Grid  Cost: %.4fs(%.4f %%)\n", gridCost, 100.f * gridCost / frameCost);
			printf("Press Cost: %.4fs(%.4f %%)\n", pressCost, 100.f * pressCost / frameCost);
			printf("Force Cost: %.4fs(%.4f %%)\n", forceCost, 100.f * forceCost / frameCost);
			printf("Adv   Cost: %.4fs(%.4f %%)\n", advCost, 100.f * advCost / frameCost);
		}
	protected:
		/** 初始化系统
		*/
		void _init(unsigned int maxPointCounts, const Box& wallBox, const Box& initFluidBox, const float4& gravity);
		/** 计算相邻关系 */
		void _resetNeighbor(void);
		/** 计算领域关系、密度、压强 */
		void _computePressure(void);
		/** 计算加速度 */
		void _computeForce(void);
		/** 移动粒子*/
		void _advance(void);
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

	protected:
		PointBuffer m_pointBuffer;
		GridContainer m_gridContainer;
		NeighborTable m_neighborTable;

		// SPH Kernel
		float m_kernelPoly6;
		float m_kernelSpiky;
		float m_kernelViscosity;

		//Other Parameters
		float m_unitScale;
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

		//performace Param
		float frameCost;
		float gridCost, pressCost, forceCost, advCost;
	public:
		FluidSystem(){
			m_unitScale = 0.004f;			// 尺寸单位
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
