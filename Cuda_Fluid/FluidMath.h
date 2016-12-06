#pragma once
//#define UN(type, a, b) union{ type a, b; }
//#define US4(type, sname) struct sname{sname(type x=0, type y=0, type z = 0, type w = 0):x(x), y(y), z(z), w(w){};  UN(type, x, r);UN(type, y, g); UN(type, z, b); UN(type, w, a); }
//US4(int, int4);
//US4(float, float4);
//

namespace SPH{
	const float PI = acosf(-1);
	template <class T>
	struct vec4{
		union{
			T x, r;
		};
		union{
			T y, b;
		};
		union{
			T z, g;
		};
		union{
			T w, a;
		};
		vec4(T x = 0, T y = 0, T z = 0, T w = 0){
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		vec4(const vec4<T>& r){
			this->x = r.x;
			this->y = r.y;
			this->z = r.z;
			this->w = r.w;
		}
		T dot(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return r.x*l.x + r.y*l.y + r.z*l.z + r.w*l.w;
		}
		vec4<T> operator*(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return return vec4<T>(l.x * r.x, l.y * r.y, l.z * r.z, l.w * r.w);
		}
		vec4<T> operator/(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return vec4<T>(l.x/r.x, l.y/r.y, l.z/r.z, l.w/r.w);
		}
		vec4<T> operator*(const float& r) const{
			const vec4<T>& l = *this;
			return vec4<T>(l.x*r, l.y*r, l.z*r, l.w*r);
		}
		vec4<T> operator/(const float& r) const{
			const vec4<T>& l = *this;
			return l*(1.0f / r);
		}
		vec4<T> operator-() const {
			const vec4<T>& r = *this;
			return vec4<T>(-r.x, -r.y, -r.z, -r.w);
		}
		vec4<T> operator+(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return vec4<T>(r.x + l.x, r.y + l.y, r.z + l.z, r.w + l.w);
		}
		vec4<T> operator-(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return l + (-r);
		}
		template <class R>
		vec4<T> operator=(const vec4<R>& r){
			vec4<T>& l = *this;
			l.x = (T)r.x;
			l.y = (T)r.y;
			l.z = (T)r.z;
			l.w = (T)r.w;
			return l;
		}
		bool operator==(const vec4<T>& r) const{
			const vec4<T>& l = *this;
			return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
		}
	};
	typedef vec4<int> int4;
	typedef vec4<float> float4;
	struct Box{
		float4 minpos;
		float4 maxpos;
		Box(){}
		Box(const float4& minpos, const float4& maxpos) :minpos(minpos), maxpos(maxpos){};
	};
	//float mul(float4 r, float4 l);
	//float4 add(float4 r, float4 l);
}