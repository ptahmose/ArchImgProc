/*----------------------------------------------------------------------------
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
----------------------------------------------------------------------------*/

#pragma once

#include <exception>
#include <vector>
#include <memory>
#include <functional>

namespace ArchImgProc
{
	namespace Internal
	{
		class CLSDNewNullStopWatch
		{
		public:
			void Initialize() {}
			void startTimer() {}
			void stopTimer() {}
			double getElapsedTime() { return 0; }
		};

		struct LSDNewPerfInfo
		{
			double	time1;	// compute gradient of source image
			double  time2;  // compute histogram of gradient values
			double	time3;	// make the list of pixels (almost) ordered by norm value
			double	time4;  // search for line segments 
		};

		template <typename imgFLT, typename calcFLT, typename tStopWatch = CLSDNewNullStopWatch>
		class LSDNew
		{
		private:
			class BitVector
			{
			private:
				unsigned char* pBuffer;
				int width, height;
				int stride;
			public:
				BitVector() = delete;
				BitVector(int width, int height, bool initialValue) : width(width), height(height), stride((width + 7) / 8)
				{
					this->pBuffer = (unsigned char*)malloc(this->stride * height);
					memset(this->pBuffer, initialValue == true ? 0xff : 0, this->stride*height);
				}

				~BitVector()
				{
					free(this->pBuffer);
				}

				int Width() const { return this->width; }
				int Height() const { return this->height; }

				bool IsSet(int x, int y) const
				{
					int offset = x / 8 + y*this->stride;
					int shift = x % 8;
					unsigned char m = 1 << shift;
					return (this->pBuffer[offset] & m) ? true : false;
				}

				void Set(int x, int y)
				{
					int offset = x / 8 + y*this->stride;
					int shift = x % 8;
					unsigned char m = 1 << shift;
					this->pBuffer[offset] |= m;
				}

				void Clear(int x, int y)
				{
					int offset = x / 8 + y*this->stride;
					int shift = x % 8;
					unsigned char m = 1 << shift;
					this->pBuffer[offset] &= ~m;
				}
			};

			typedef BitVector UsedStore;
		private:
			/// Base class for implementing a sampler.
			class SamplerBase
			{
			protected:
				const void* ptr;
				int width, height, stride;
				SamplerBase(const void* ptr, int width, int height, int stride)
					: ptr(ptr), width(width), height(height), stride(stride)
				{}
			public:
				int XSize() const { return this->width; }
				int YSize() const { return this->height; }
			};

			/// Sampler which operates on float-bitmaps.
			template <typename tFlt>
			class SamplerFloat :public SamplerBase
			{
			public:
				SamplerFloat(const float* ptr, int width, int height, int stride)
					: SampleBase(ptr, width, height, stride)
				{}

				void getCom1Com2(int x, int y, tFlt& com1, tFlt& com2) const
				{
					const char* pData = (((const char*)this->ptr) + (this->stride*y) + x * sizeof(float));
					auto p1 = *((const float*)(pData + this->stride + sizeof(float)));
					auto p2 = *((const float*)pData);
					com1 = (tFlt)(p1 - p2);
					p1 = *((const float*)(pData + sizeof(float)));
					p2 = *((const float*)(pData + this->stride));
					com2 = (tFlt)(p1 - p2);
				}
			};
			template <typename tFlt>
			class SamplerDouble :public SamplerBase
			{
			public:
				SamplerDouble(const double* ptr, int width, int height, int stride)
					: SamplerBase(ptr, width, height, stride)
				{}

				void getCom1Com2(int x, int y, tFlt& com1, tFlt& com2) const
				{
					const char* pData = (((const char*)this->ptr) + (this->stride*y) + x * sizeof(double));
					auto p1 = *((const double*)(pData + this->stride + sizeof(double)));
					auto p2 = *((const double*)pData);
					com1 = (tFlt)(p1 - p2);
					p1 = *((const double*)(pData + sizeof(double)));
					p2 = *((const double*)(pData + this->stride));
					com2 = (tFlt)(p1 - p2);
				}
			};
			template <typename tFlt>
			class SamplerARGB :public SamplerBase
			{
			public:
				SamplerARGB(const unsigned char* ptr, int width, int height, int stride)
					: SamplerBase(ptr, width, height, stride)
				{}

				void getCom1Com2(int x, int y, tFlt& com1, tFlt& com2) const
				{
					const unsigned char* pData = (((const unsigned char*)this->ptr) + (this->stride*y) + x * 4);

					const unsigned char* pData1 = pData + this->stride + 4;
					const unsigned char* pData2 = pData;

					int p1 = ((int)pData1[0]) + ((int)pData1[1]) + ((int)pData1[2]);
					int p2 = ((int)pData2[0]) + ((int)pData2[1]) + ((int)pData2[2]);

					com1 = (tFlt)(p1 - p2);
					pData1 = pData + 4;
					pData2 = pData + this->stride;
					p1 = ((int)pData1[0]) + ((int)pData1[1]) + ((int)pData1[2]);
					p2 = ((int)pData2[0]) + ((int)pData2[1]) + ((int)pData2[2]);
					com2 = (tFlt)(p1 - p2);
				}
			};
			template <typename tFlt>
			class SamplerBGR :public SamplerBase
			{
			public:
				SamplerBGR(const unsigned char* ptr, int width, int height, int stride)
					: SamplerBase(ptr, width, height, stride)
				{}

				void getCom1Com2(int x, int y, tFlt& com1, tFlt& com2) const
				{
					const unsigned char* pData = (((const unsigned char*)this->ptr) + (this->stride*y) + x * 3);

					const unsigned char* pData1 = pData + this->stride + 3;
					const unsigned char* pData2 = pData;

					int p1 = ((int)pData1[0]) + ((int)pData1[1]) + ((int)pData1[2]);
					int p2 = ((int)pData2[0]) + ((int)pData2[1]) + ((int)pData2[2]);

					com1 = (tFlt)(p1 - p2);
					pData1 = pData + 3;
					pData2 = pData + this->stride;
					p1 = ((int)pData1[0]) + ((int)pData1[1]) + ((int)pData1[2]);
					p2 = ((int)pData2[0]) + ((int)pData2[1]) + ((int)pData2[2]);
					com2 = (tFlt)(p1 - p2);
				}
			};
			template <typename tFlt>
			class SamplerGray8 :public SamplerBase
			{
			public:
				SamplerGray8(const unsigned char* ptr, int width, int height, int stride)
					: SamplerBase(ptr, width, height, stride)
				{}

				void getCom1Com2(int x, int y, tFlt& com1, tFlt& com2) const
				{
					const unsigned char* pData = (((const unsigned char*)this->ptr) + (this->stride*y) + x);

					const unsigned char* pData1 = pData + this->stride + 1;
					const unsigned char* pData2 = pData;

					int p1 = ((int)pData1[0]);
					int p2 = ((int)pData2[0]);

					com1 = (tFlt)(p1 - p2);
					pData1 = pData + 1;
					pData2 = pData + this->stride;
					p1 = ((int)pData1[0]);
					p2 = ((int)pData2[0]);
					com2 = (tFlt)(p1 - p2);
				}
			};

			constexpr static calcFLT DefaultQuant() { return static_cast<calcFLT>(2.0); }       /* Bound to the quantization error on the gradient norm.	*/
			constexpr static calcFLT DefaultAng_th() { return static_cast<calcFLT>(22.5); }     /* Gradient angle tolerance in degrees.					    */
			constexpr static calcFLT DefaultLog_eps() { return 0; }							    /* Detection threshold: -log10(NFA) > log_eps				*/
			constexpr static calcFLT DefaultDensity_th() { return  static_cast<calcFLT>(0.7); } /* Minimal density of region points in rectangle.			*/
			constexpr static int DefaultN_bins() { return 1024; }
		public:
			typedef ArchImgProc::LineSegment<calcFLT> LineSegment;

			static std::vector<LineSegment> LSD(
				const float* img,
				int width,
				int height,
				int stride,
				const float* ptrScaleFactors = nullptr,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				SamplerFloat<calcFLT> sf(img, width, height, stride);
				return LineSegmentDetection<SamplerFloat<calcFLT>>(
					sf,
					DefaultQuant(),
					DefaultAng_th(),
					DefaultLog_eps(),
					DefaultDensity_th(),
					DefaultN_bins(),
					ptrScaleFactors,
					perfInfo);
			}

			static std::vector<LineSegment> LSD(
				const double* img,
				int width,
				int height,
				int stride,
				const double* ptrScaleFactors = nullptr,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				SamplerDouble<calcFLT> sf(img, width, height, stride);
				return LineSegmentDetection<SamplerDouble<calcFLT>>(
					sf,
					DefaultQuant(),
					DefaultAng_th(),
					DefaultLog_eps(),
					DefaultDensity_th(),
					DefaultN_bins(),
					ptrScaleFactors,
					perfInfo);
			}

			static std::vector<LineSegment> LSD_RGBA(
				const unsigned char* img,
				int width,
				int height,
				int stride,
				const calcFLT* ptrScaleFactors = nullptr,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				SamplerARGB<calcFLT> sf(img, width, height, stride);
				return LineSegmentDetection<SamplerARGB<calcFLT>>(
					sf,
					DefaultQuant(),
					DefaultAng_th(),
					DefaultLog_eps(),
					DefaultDensity_th(),
					DefaultN_bins(),
					ptrScaleFactors,
					perfInfo);
			}

			static std::vector<LineSegment> LSD_BGR(
				const unsigned char* img,
				int width,
				int height,
				int stride,
				const calcFLT* ptrScaleFactors = nullptr,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				SamplerBGR<calcFLT> sf(img, width, height, stride);
				return LineSegmentDetection<SamplerBGR<calcFLT>>(
					sf,
					DefaultQuant(),
					DefaultAng_th(),
					DefaultLog_eps(),
					DefaultDensity_th(),
					DefaultN_bins(),
					ptrScaleFactors,
					perfInfo);
			}
			static std::vector<LineSegment> LSD_Gray8(
				const unsigned char* img,
				int width,
				int height,
				int stride,
				const calcFLT* ptrScaleFactors = nullptr,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				SamplerGray8<calcFLT> sf(img, width, height, stride);
				return LineSegmentDetection<SamplerGray8<calcFLT>>(
					sf,
					DefaultQuant(),
					/*2**/DefaultAng_th(),
					/*2**/DefaultLog_eps(),
					DefaultDensity_th(),
					DefaultN_bins(),
					ptrScaleFactors,
					perfInfo);
			}
		private:
			class image_deleter
			{
			private:
				void* p;
			public:
				image_deleter(void* p) : p(p) {};
				~image_deleter() { free(this->p); }
			};

		private:
			static constexpr calcFLT Constants_PI() { return (calcFLT)3.14159265358979323846; }
			static constexpr calcFLT Constants_2PI() { return (calcFLT)(2 * 3.14159265358979323846); }
			static constexpr calcFLT Constants_3_2_PI() { return (calcFLT)(3 * 3.14159265358979323846 / 2); }
			static constexpr calcFLT Constants_Log10_Of_11() { return (calcFLT)1.04139268515822504075020; /*log10 (11) */ }
			static constexpr calcFLT Constants_LN10() { return (calcFLT)2.30258509299404568402; }

			static calcFLT pow6(calcFLT x) { calcFLT v = x*x*x; return v*v; }
			static calcFLT pow5(calcFLT x) { calcFLT v = x*x; return v*v*x; }
			static calcFLT pow4(calcFLT x) { calcFLT v = x*x; return v*v; }
			static calcFLT pow3(calcFLT x) { return x*x*x; }
			static calcFLT pow2(calcFLT x) { return x*x; }
			static calcFLT pow_n(calcFLT x, int n)
			{
				switch (n)
				{
				case 6:return pow6(x);
				case 5:return pow5(x);
				case 4:return pow4(x);
				case 3:return pow3(x);
				case 2:return pow2(x);
				case 1:return x;
				case 0:return 1;
				}

				throw std::runtime_error("Exponent must be >=0 and <=6,");
			}
		private:
			static void SetToNOTDEF(imgFLT& val) { val = -1024; }
			static bool IsNOTDEF(const imgFLT val) { return val == -1024 ? true : false; }

			struct image_double
			{
				imgFLT * data;
				unsigned int xsize, ysize;
			};

			struct image_char
			{
				unsigned char* data;
				unsigned int xsize, ysize;
			};

			struct const_image_double
			{
				const imgFLT * data;
				unsigned int xsize, ysize;
			};

			struct coorlist
			{
				int x, y;
				struct coorlist * next;
			};

			struct rect_iter
			{
				calcFLT vx[4];  /* rectangle's corner X coordinates in circular order */
				calcFLT vy[4];  /* rectangle's corner Y coordinates in circular order */
				calcFLT ys, ye;  /* start and end Y values of current 'column' */
				int x, y;       /* coordinates of currently explored pixel */
			};

			struct rect
			{
				calcFLT x1, y1, x2, y2;  /* first and second point of the line segment */
				calcFLT width;        /* rectangle width */
				calcFLT x, y;          /* center of the rectangle */
				calcFLT theta;        /* angle */
				calcFLT dx, dy;        /* (dx,dy) is vector oriented as the line segment */
				calcFLT prec;         /* tolerance angle */
				calcFLT p;            /* probability of a point with angle within 'prec' */
			};

			struct point
			{
				int x, y;
			};

			template <class tSampler>
			static std::vector<LineSegment> LineSegmentDetection(
				const tSampler& src,
				calcFLT quant,
				calcFLT ang_th,
				calcFLT log_eps,
				calcFLT density_th,
				int n_bins,
				const calcFLT* ptrFactors,
				LSDNewPerfInfo* perfInfo = nullptr)
			{
				std::vector<LineSegment> lslist;

				/* check parameters */
				if (src.XSize() <= 0 || src.YSize() <= 0) throw std::invalid_argument("invalid image input.");
				if (quant < 0.0) throw std::invalid_argument("'quant' value must be positive.");
				if (ang_th <= 0.0 || ang_th >= 180.0) throw std::invalid_argument("'ang_th' value must be in the range (0,180).");
				if (density_th < 0.0 || density_th > 1.0) throw std::invalid_argument("'density_th' value must be in the range [0,1].");
				if (n_bins <= 0) throw std::invalid_argument("'n_bins' value must be positive.");

				/* angle tolerance */
				calcFLT prec = Constants_PI() * ang_th / 180;
				calcFLT p = ang_th / 180;
				calcFLT rho = quant / sin(prec); /* gradient magnitude threshold */

												 //const_image_double image = { img, X, Y };
				coorlist * list_p; void * mem_p;
				image_double modgrad;
				image_double angles = ll_angle<tSampler>(src, rho, &list_p, &mem_p, &modgrad, static_cast<unsigned int>(n_bins), perfInfo);
				image_deleter angles_deleter(angles.data); image_deleter modgrad_deleter(modgrad.data);
				image_deleter mem_p_deleter(mem_p);
				unsigned int xsize = angles.xsize;
				unsigned int ysize = angles.ysize;

				/* Number of Tests - NT

				The theoretical number of tests is Np.(XY)^(5/2)
				where X and Y are number of columns and rows of the image.
				Np corresponds to the number of angle precisions considered.
				As the procedure 'rect_improve' tests 5 times to halve the
				angle precision, and 5 more times after improving other factors,
				11 different precision values are potentially tested. Thus,
				the number of tests is
				11 * (X*Y)^(5/2)
				whose logarithm value is
				log10(11) + 5/2 * (log10(X) + log10(Y)).
				*/
				calcFLT logNT = 5 * (log10((calcFLT)xsize) + log10((calcFLT)ysize)) / 2 + Constants_Log10_Of_11() /*log10(11)*/;
				int min_reg_size = (int)(-logNT / log10(p)); /* minimal number of points in region
															 that can give a meaningful event */


				UsedStore used2(xsize, ysize, false);

				std::unique_ptr<point, std::function<void(point*)>> reg(static_cast<point*>(calloc((xsize*ysize), sizeof(point))), [](point* ptr) {free(ptr); });

				int ls_count = 0;

				tStopWatch sw;
				if (perfInfo != nullptr)
				{
					sw.Initialize();
					sw.startTimer();
				}

				/* search for line segments */
				for (; list_p != nullptr; list_p = list_p->next)
					if (used2.IsSet(list_p->x, list_p->y) == false &&
						!IsNOTDEF(angles.data[list_p->x + list_p->y * angles.xsize]))
						/* there is no risk of double comparison problems here
						because we are only interested in the exact NOTDEF value */
					{
						int reg_size;
						calcFLT reg_angle;
						/* find the region of connected point and ~equal angle */
						region_grow(list_p->x, list_p->y, angles, reg.get(), &reg_size, &reg_angle, used2, prec);

						/* reject small regions */
						if (reg_size < min_reg_size) continue;

						/* construct rectangular approximation for the region */
						rect rec;
						region2rect(reg.get(), reg_size, modgrad, reg_angle, prec, p, &rec);

						/* Check if the rectangle exceeds the minimal density of
						region points. If not, try to improve the region.
						The rectangle will be rejected if the final one does
						not fulfill the minimal density condition.
						This is an addition to the original LSD algorithm published in
						"LSD: A Fast Line Segment Detector with a False Detection Control"
						by R. Grompone von Gioi, J. Jakubowicz, J.M. Morel, and G. Randall.
						The original algorithm is obtained with density_th = 0.0.
						*/
						if (!refine(reg.get(), &reg_size, modgrad, reg_angle, prec, p, &rec, used2, angles, density_th))
							continue;

						/* compute NFA value */
						calcFLT log_nfa = rect_improve(rec, angles, logNT, log_eps);
						if (log_nfa <= log_eps) continue;

						/* A New Line Segment was found! */
						++ls_count;  /* increase line segment counter */

									 /*
									 The gradient was computed with a 2x2 mask, its value corresponds to
									 points with an offset of (0.5,0.5), that should be added to output.
									 The coordinates origin is at the center of pixel (0,0).
									 */
						rec.x1 += (calcFLT)0.5; rec.y1 += (calcFLT)0.5;
						rec.x2 += (calcFLT)0.5; rec.y2 += (calcFLT)0.5;

						if (ptrFactors != nullptr)
						{
							lslist.push_back(LineSegment{ ptrFactors[0] * rec.x1, ptrFactors[1] * rec.y1, ptrFactors[0] * rec.x2, ptrFactors[1] * rec.y2, rec.width, rec.p, log_nfa });
						}
						else
						{
							lslist.push_back(LineSegment{ rec.x1, rec.y1, rec.x2, rec.y2, rec.width, rec.p, log_nfa });
						}
					}

				if (perfInfo != nullptr)
				{
					sw.stopTimer();
					perfInfo->time4 = sw.getElapsedTime();
				}

				return lslist;
			}

			static calcFLT rect_improve(rect& rec, const image_double& angles, calcFLT logNT, calcFLT log_eps)
			{
				calcFLT log_nfa_new;
				const calcFLT delta = (calcFLT)0.5;
				const calcFLT delta_2 = delta / 2;

				calcFLT log_nfa = rect_nfa(rec, angles, logNT);

				if (log_nfa > log_eps) return log_nfa;

				/* try finer precisions */
				rect r = rec;
				for (int n = 0; n < 5; n++)
				{
					r.p /= 2.0;
					r.prec = r.p * Constants_PI();
					log_nfa_new = rect_nfa(r, angles, logNT);
					if (log_nfa_new > log_nfa)
					{
						log_nfa = log_nfa_new;
						rec = r;
					}
				}

				if (log_nfa > log_eps) return log_nfa;

				/* try to reduce width */
				r = rec;
				for (int n = 0; n < 5; n++)
				{
					if ((r.width - delta) >= (calcFLT)0.5)
					{
						r.width -= delta;
						log_nfa_new = rect_nfa(r, angles, logNT);
						if (log_nfa_new > log_nfa)
						{
							rec = r;
							log_nfa = log_nfa_new;
						}
					}
				}

				if (log_nfa > log_eps) return log_nfa;

				/* try to reduce one side of the rectangle */
				//rect_copy(rec, &r);
				r = rec;
				for (int n = 0; n < 5; n++)
				{
					if ((r.width - delta) >= (calcFLT)0.5)
					{
						r.x1 += -r.dy * delta_2;
						r.y1 += r.dx * delta_2;
						r.x2 += -r.dy * delta_2;
						r.y2 += r.dx * delta_2;
						r.width -= delta;
						log_nfa_new = rect_nfa(r, angles, logNT);
						if (log_nfa_new > log_nfa)
						{
							rec = r;
							log_nfa = log_nfa_new;
						}
					}
				}

				if (log_nfa > log_eps) return log_nfa;

				/* try to reduce the other side of the rectangle */
				r = rec;
				for (int n = 0; n < 5; n++)
				{
					if ((r.width - delta) >= (calcFLT)0.5)
					{
						r.x1 -= -r.dy * delta_2;
						r.y1 -= r.dx * delta_2;
						r.x2 -= -r.dy * delta_2;
						r.y2 -= r.dx * delta_2;
						r.width -= delta;
						log_nfa_new = rect_nfa(r, angles, logNT);
						if (log_nfa_new > log_nfa)
						{
							rec = r;
							log_nfa = log_nfa_new;
						}
					}
				}

				if (log_nfa > log_eps) return log_nfa;

				/* try even finer precisions */
				r = rec;
				for (int n = 0; n < 5; n++)
				{
					r.p /= 2;
					r.prec = r.p * Constants_PI();
					log_nfa_new = rect_nfa(r, angles, logNT);
					if (log_nfa_new > log_nfa)
					{
						log_nfa = log_nfa_new;
						rec = r;
					}
				}

				return log_nfa;
			}

			static calcFLT rect_nfa(const rect& rec, const image_double& angles, calcFLT logNT)
			{
				rect_iter i;
				int pts = 0;
				int alg = 0;

				/* check parameters */
				if (angles.data == nullptr) throw std::invalid_argument("rect_nfa: invalid 'angles'.");

				/* compute the total number of pixels and of aligned points in 'rec' */
				for (i = ri_ini(rec); !ri_end(i); ri_inc(i)) /* rectangle iterator */
					if (i.x >= 0 && i.y >= 0 &&
						i.x < (int)angles.xsize && i.y < (int)angles.ysize)
					{
						++pts; /* total number of pixels counter */
						if (isaligned(i.x, i.y, angles, rec.theta, rec.prec))
							++alg; /* aligned points counter */
					}

				return nfa(pts, alg, rec.p, logNT); /* compute NFA value */
			}


			static calcFLT nfa(int n, int k, calcFLT p, calcFLT logNT)
			{
				//const int TABSIZE = 100000;
				//static calcFLT  inv[TABSIZE];   /* table to keep computed inverse values */
				// TODO...

				calcFLT  tolerance = static_cast<calcFLT>(0.1);       /* an error of 10% in the result is accepted */
				calcFLT  log1term, term, bin_term, mult_term, bin_tail, err, p_term;
				int i;

#if _DEBUG
				/* check parameters */
				if (n < 0 || k<0 || k>n || p <= 0.0 || p >= 1.0)
					throw std::invalid_argument("nfa: wrong n, k or p values.");
#endif

				/* trivial cases */
				if (n == 0 || k == 0) return -logNT;
				if (n == k) return -logNT - n * log10(p);

				/* probability term */
				p_term = p / (1 - p);

				/* compute the first term of the series */
				/*
				binomial_tail(n,k,p) = sum_{i=k}^n bincoef(n,i) * p^i * (1-p)^{n-i}
				where bincoef(n,i) are the binomial coefficients.
				But
				bincoef(n,k) = gamma(n+1) / ( gamma(k+1) * gamma(n-k+1) ).
				We use this to compute the first term. Actually the log of it.
				*/
				log1term = log_gamma((calcFLT)(n + 1)) - log_gamma((calcFLT)(k + 1))
					- log_gamma((calcFLT)((n - k) + 1))
					+ k * log(p) + (n - k) * log(1 - p);
				term = exp(log1term);

				/* in some cases no more computations are needed */
				if (double_equal(term, 0))              /* the first term is almost zero */
				{
					if ((calcFLT)k > (calcFLT)n * p)     /* at begin or end of the tail?  */
						return -log1term / Constants_LN10() - logNT;  /* end: use just the first term  */
					else
						return -logNT;                      /* begin: the tail is roughly 1  */
				}

				/* compute more terms if needed */
				bin_tail = term;
				for (i = k + 1; i <= n; i++)
				{
					/*
					As
					term_i = bincoef(n,i) * p^i * (1-p)^(n-i)
					and
					bincoef(n,i)/bincoef(n,i-1) = n-1+1 / i,
					then,
					term_i / term_i-1 = (n-i+1)/i * p/(1-p)
					and
					term_i = term_i-1 * (n-i+1)/i * p/(1-p).
					1/i is stored in a table as they are computed,
					because divisions are expensive.
					p/(1-p) is computed only once and stored in 'p_term'.
					*/
					/*bin_term = (calcFLT)(n - i + 1) * (i < TABSIZE ?
					(inv[i] != 0 ? inv[i] : (inv[i] = 1 / (calcFLT)i)) :
					1 / (calcFLT)i);*/
					bin_term = (n - i + 1) / static_cast<calcFLT>(i);

					mult_term = bin_term * p_term;
					term *= mult_term;
					bin_tail += term;
					if (bin_term < 1)
					{
						/* When bin_term<1 then mult_term_j<mult_term_i for j>i.
						Then, the error on the binomial tail when truncated at
						the i term can be bounded by a geometric series of form
						term_i * sum mult_term_i^j.                            */
						err = term * ((1 - pow(mult_term, (calcFLT)(n - i + 1))) /
							(1 - mult_term) - 1);

						/* One wants an error at most of tolerance*final_result, or:
						tolerance * abs(-log10(bin_tail)-logNT).
						Now, the error that can be accepted on bin_tail is
						given by tolerance*final_result divided by the derivative
						of -log10(x) when x=bin_tail. that is:
						tolerance * abs(-log10(bin_tail)-logNT) / (1/bin_tail)
						Finally, we truncate the tail if the error is less than:
						tolerance * abs(-log10(bin_tail)-logNT) * bin_tail        */
						if (err < tolerance * fabs(-log10(bin_tail) - logNT) * bin_tail) break;
					}
				}
				return -log10(bin_tail) - logNT;
			}

			static calcFLT log_gamma(calcFLT x)
			{
				if (x > 15)
				{
					// Windschitl method
					return static_cast<calcFLT>(0.918938533204673) + (x - static_cast<calcFLT>(0.5))*log(x) - x + static_cast<calcFLT>(0.5)*x*log(x*sinh(1 / x) + 1 / (810 * pow6(x)));
				}
				else
				{
					// Lanczos approximation
					static const calcFLT q[7] = { static_cast<calcFLT>(75122.6331530), static_cast<calcFLT>(80916.6278952), static_cast<calcFLT>(36308.2951477), static_cast<calcFLT>(8687.24529705), static_cast<calcFLT>(1168.92649479), static_cast<calcFLT>(83.8676043424), static_cast<calcFLT>(2.50662827511) };
					calcFLT a = (x + static_cast<calcFLT>(0.5)) * log(x + static_cast<calcFLT>(5.5)) - (x + static_cast<calcFLT>(5.5));
					calcFLT b = 0;

					for (int n = 0; n < 7; n++)
					{
						a -= log(x + n);
						b += q[n] * pow_n(x, n);
					}

					return a + log(b);
				}
			}

			static rect_iter ri_ini(const rect& r)
			{
				calcFLT vx[4], vy[4];
				int offset;
				rect_iter  i;

				/* build list of rectangle corners ordered
				in a circular way around the rectangle */
				vx[0] = r.x1 - r.dy * r.width / 2;
				vy[0] = r.y1 + r.dx * r.width / 2;
				vx[1] = r.x2 - r.dy * r.width / 2;
				vy[1] = r.y2 + r.dx * r.width / 2;
				vx[2] = r.x2 + r.dy * r.width / 2;
				vy[2] = r.y2 - r.dx * r.width / 2;
				vx[3] = r.x1 + r.dy * r.width / 2;
				vy[3] = r.y1 - r.dx * r.width / 2;

				/* compute rotation of index of corners needed so that the first
				point has the smaller x.

				if one side is vertical, thus two corners have the same smaller x
				value, the one with the largest y value is selected as the first.
				*/
				if (r.x1 < r.x2 && r.y1 <= r.y2) offset = 0;
				else if (r.x1 >= r.x2 && r.y1 < r.y2) offset = 1;
				else if (r.x1 > r.x2 && r.y1 >= r.y2) offset = 2;
				else offset = 3;

				/* apply rotation of index. */
				for (int n = 0; n < 4; n++)
				{
					i.vx[n] = vx[(offset + n) % 4];
					i.vy[n] = vy[(offset + n) % 4];
				}

				/* Set an initial condition.

				The values are set to values that will cause 'ri_inc' (that will
				be called immediately) to initialize correctly the first 'column'
				and compute the limits 'ys' and 'ye'.

				'y' is set to the integer value of vy[0], the starting corner.

				'ys' and 'ye' are set to very small values, so 'ri_inc' will
				notice that it needs to start a new 'column'.

				The smallest integer coordinate inside of the rectangle is
				'ceil(vx[0])'. The current 'x' value is set to that value minus
				one, so 'ri_inc' (that will increase x by one) will advance to
				the first 'column'.
				*/
				i.x = (int)ceil(i.vx[0]) - 1;
				i.y = (int)ceil(i.vy[0]);
				i.ys = i.ye = std::numeric_limits<calcFLT>::lowest();

				/* advance to the first pixel */
				ri_inc(i);

				return i;
			}

			static void ri_inc(rect_iter&  i)
			{
				/* if not at end of exploration,
				increase y value for next pixel in the 'column' */
				if (!ri_end(i)) ++i.y;

				/* if the end of the current 'column' is reached,
				and it is not the end of exploration,
				advance to the next 'column' */
				while ((calcFLT)(i.y) > i.ye && !ri_end(i))
				{
					/* increase x, next 'column' */
					++i.x;

					/* if end of exploration, return */
					if (ri_end(i)) return;

					/* update lower y limit (start) for the new 'column'.

					We need to interpolate the y value that corresponds to the
					lower side of the rectangle. The first thing is to decide if
					the corresponding side is

					vx[0],vy[0] to vx[3],vy[3] or
					vx[3],vy[3] to vx[2],vy[2]

					Then, the side is interpolated for the x value of the
					'column'. But, if the side is vertical (as it could happen if
					the rectangle is vertical and we are dealing with the first
					or last 'columns') then we pick the lower value of the side
					by using 'inter_low'.
					*/
					if (static_cast<calcFLT>(i.x) < i.vx[3])
						i.ys = inter_low(static_cast<calcFLT>(i.x), i.vx[0], i.vy[0], i.vx[3], i.vy[3]);
					else
						i.ys = inter_low(static_cast<calcFLT>(i.x), i.vx[3], i.vy[3], i.vx[2], i.vy[2]);

					/* update upper y limit (end) for the new 'column'.

					We need to interpolate the y value that corresponds to the
					upper side of the rectangle. The first thing is to decide if
					the corresponding side is

					vx[0],vy[0] to vx[1],vy[1] or
					vx[1],vy[1] to vx[2],vy[2]

					Then, the side is interpolated for the x value of the
					'column'. But, if the side is vertical (as it could happen if
					the rectangle is vertical and we are dealing with the first
					or last 'columns') then we pick the lower value of the side
					by using 'inter_low'.
					*/
					if (static_cast<calcFLT>(i.x) < i.vx[1])
						i.ye = inter_hi(static_cast<calcFLT>(i.x), i.vx[0], i.vy[0], i.vx[1], i.vy[1]);
					else
						i.ye = inter_hi(static_cast<calcFLT>(i.x), i.vx[1], i.vy[1], i.vx[2], i.vy[2]);

					/* new y */
					i.y = (int)ceil(i.ys);
				}
			}

			static int ri_end(const rect_iter& i)
			{
				/* if the current x value is larger than the largest
				x value in the rectangle (vx[2]), we know the full
				exploration of the rectangle is finished. */
				return (calcFLT)(i.x) > i.vx[2];
			}

			static calcFLT inter_hi(calcFLT x, calcFLT x1, calcFLT y1, calcFLT x2, calcFLT y2)
			{
#if _DEBUG
				/* check parameters */
				if (x1 > x2 || x < x1 || x > x2)
					throw std::invalid_argument("inter_hi: unsuitable input, 'x1>x2' or 'x<x1' or 'x>x2'.");
#endif

				/* interpolation */
				if (double_equal(x1, x2) && y1 < y2) return y2;
				if (double_equal(x1, x2) && y1 > y2) return y1;
				return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
			}

			static calcFLT inter_low(calcFLT x, calcFLT x1, calcFLT y1, calcFLT x2, calcFLT y2)
			{
#if _DEBUG
				/* check parameters */
				if (x1 > x2 || x < x1 || x > x2)
					throw std::invalid_argument("inter_low: unsuitable input, 'x1>x2' or 'x<x1' or 'x>x2'.");
#endif

				/* interpolation */
				if (double_equal(x1, x2) && y1 < y2) return y1;
				if (double_equal(x1, x2) && y1 > y2) return y2;
				return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
			}

			static bool refine(point * reg, int * reg_size, const image_double& modgrad,
				calcFLT reg_angle, calcFLT prec, calcFLT p, struct rect * rec,
				UsedStore& used, const image_double& angles, calcFLT density_th)
			{
				calcFLT angle, ang_d, mean_angle, tau, density, xc, yc, ang_c, sum, s_sum;
				int i, n;

#if _DEBUG
				/* check parameters */
				if (reg == nullptr) throw std::invalid_argument("refine: invalid pointer 'reg'.");
				if (reg_size == nullptr) throw std::invalid_argument("refine: invalid pointer 'reg_size'.");
				if (prec < 0) throw std::invalid_argument("refine: 'prec' must be positive.");
				if (rec == nullptr) throw std::invalid_argument("refine: invalid pointer 'rec'.");
				if (angles.data == nullptr)
					throw std::invalid_argument("refine: invalid image 'angles'.");
#endif

				/* compute region points density */
				density = ((calcFLT)*reg_size) / (dist(rec->x1, rec->y1, rec->x2, rec->y2) * rec->width);

				/* if the density criterion is satisfied there is nothing to do */
				if (density >= density_th) return true;

				/*------ First try: reduce angle tolerance ------*/

				/* compute the new mean angle and tolerance */
				xc = (calcFLT)reg[0].x;
				yc = (calcFLT)reg[0].y;
				ang_c = angles.data[reg[0].x + reg[0].y * angles.xsize];
				sum = s_sum = 0;
				n = 0;
				for (i = 0; i < *reg_size; i++)
				{
					used.Clear(reg[i].x, reg[i].y);
					if (dist(xc, yc, (calcFLT)reg[i].x, (calcFLT)reg[i].y) < rec->width)
					{
						angle = angles.data[reg[i].x + reg[i].y * angles.xsize];
						ang_d = angle_diff_signed(angle, ang_c);
						sum += ang_d;
						s_sum += ang_d * ang_d;
						++n;
					}
				}
				mean_angle = sum / n;
				tau = 2 * sqrt((s_sum - 2 * mean_angle * sum) / n + mean_angle*mean_angle); /* 2 * standard deviation */

																							/* find a new region from the same starting point and new angle tolerance */
				region_grow(reg[0].x, reg[0].y, angles, reg, reg_size, &reg_angle, used, tau);

				/* if the region is too small, reject */
				if (*reg_size < 2) return false;

				/* re-compute rectangle */
				region2rect(reg, *reg_size, modgrad, reg_angle, prec, p, rec);

				/* re-compute region points density */
				density = (calcFLT)*reg_size /
					(dist(rec->x1, rec->y1, rec->x2, rec->y2) * rec->width);

				/*------ Second try: reduce region radius ------*/
				if (density < density_th)
					return reduce_region_radius(reg, reg_size, modgrad, reg_angle, prec, p, rec, used, angles, density_th);

				/* if this point is reached, the density criterion is satisfied */
				return true;
			}

			static calcFLT dist(calcFLT x1, calcFLT y1, calcFLT x2, calcFLT y2)
			{
				return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
			}

			static bool reduce_region_radius(struct point * reg, int * reg_size,
				const image_double& modgrad, calcFLT reg_angle,
				calcFLT prec, calcFLT p, struct rect * rec,
				UsedStore& used, const image_double& angles,
				calcFLT density_th)
			{
				calcFLT density, _rad1, _rad2, rad, xc, yc;

#if _DEBUG
				/* check parameters */
				if (reg == nullptr) throw std::invalid_argument("reduce_region_radius: invalid pointer 'reg'.");
				if (reg_size == nullptr)
					throw std::invalid_argument("reduce_region_radius: invalid pointer 'reg_size'.");
				if (prec < 0) throw std::invalid_argument("reduce_region_radius: 'prec' must be positive.");
				if (rec == nullptr) throw std::invalid_argument("reduce_region_radius: invalid pointer 'rec'.");
				if (angles.data == NULL)
					throw std::invalid_argument("reduce_region_radius: invalid image 'angles'.");
#endif

				/* compute region points density */
				density = ((calcFLT)*reg_size) / (dist(rec->x1, rec->y1, rec->x2, rec->y2) * rec->width);

				/* if the density criterion is satisfied there is nothing to do */
				if (density >= density_th) return true;

				/* compute region's radius */
				xc = (calcFLT)reg[0].x;
				yc = (calcFLT)reg[0].y;
				_rad1 = dist(xc, yc, rec->x1, rec->y1);
				_rad2 = dist(xc, yc, rec->x2, rec->y2);
				rad = _rad1 > _rad2 ? _rad1 : _rad2;

				/* while the density criterion is not satisfied, remove farther pixels */
				while (density < density_th)
				{
					rad *= (calcFLT)0.75; /* reduce region's radius to 75% of its value */

										  /* remove points from the region and update 'used' map */
					for (int i = 0; i < *reg_size; i++)
						if (dist(xc, yc, (calcFLT)reg[i].x, (calcFLT)reg[i].y) > rad)
						{
							/* point not kept, mark it as NOTUSED */
							//used.data[reg[i].x + reg[i].y * used.xsize] = NOTUSED;
							used.Clear(reg[i].x, reg[i].y);
							/* remove point from the region */
							reg[i].x = reg[*reg_size - 1].x; /* if i==*reg_size-1 copy itself */
							reg[i].y = reg[*reg_size - 1].y;
							--(*reg_size);
							--i; /* to avoid skipping one point */
						}

					/* reject if the region is too small.
					2 is the minimal region size for 'region2rect' to work. */
					if (*reg_size < 2) return false;

					/* re-compute rectangle */
					region2rect(reg, *reg_size, modgrad, reg_angle, prec, p, rec);

					/* re-compute region points density */
					density = (calcFLT)*reg_size / (dist(rec->x1, rec->y1, rec->x2, rec->y2) * rec->width);
				}

				/* if this point is reached, the density criterion is satisfied */
				return true;
			}

			/** Build a region of pixels that share the same angle, up to a
			tolerance 'prec', starting at point (x,y).
			*/
			static void region_grow(int x, int y, const image_double& angles, struct point * reg, int * reg_size, calcFLT* reg_angle, UsedStore& used, calcFLT prec)
			{
#if _DEBUG
				/* check parameters */
				if (x < 0 || y < 0 || x >= (int)angles.xsize || y >= (int)angles.ysize)
					throw std::invalid_argument("region_grow: (x,y) out of the image.");
				if (angles.data == nullptr)
					throw std::invalid_argument("region_grow: invalid image 'angles'.");
				if (reg == nullptr) throw std::invalid_argument("region_grow: invalid 'reg'.");
				if (reg_size == nullptr) throw std::invalid_argument("region_grow: invalid pointer 'reg_size'.");
				if (reg_angle == nullptr) throw std::invalid_argument("region_grow: invalid pointer 'reg_angle'.");
#endif

				/* first point of the region */
				*reg_size = 1;
				reg[0].x = x;
				reg[0].y = y;
				*reg_angle = angles.data[x + y*angles.xsize];  /* region's angle */
				calcFLT sumdx = cos(*reg_angle);
				calcFLT sumdy = sin(*reg_angle);
				used.Set(x, y);

				/* try neighbors as new region points */
				for (int i = 0; i < *reg_size; i++)
					for (int xx = reg[i].x - 1; xx <= reg[i].x + 1; xx++)
						for (int yy = reg[i].y - 1; yy <= reg[i].y + 1; yy++)
							if (xx >= 0 && yy >= 0 && xx < used.Width() && yy < used.Height() &&
								!used.IsSet(xx, yy) &&
								isaligned(xx, yy, angles, *reg_angle, prec))
							{
								/* add point */
								used.Set(xx, yy);
								reg[*reg_size].x = xx;
								reg[*reg_size].y = yy;
								++(*reg_size);

								/* update region's angle */
								sumdx += cos(angles.data[xx + yy*angles.xsize]);
								sumdy += sin(angles.data[xx + yy*angles.xsize]);
								*reg_angle = atan2(sumdy, sumdx);
							}
			}

			/*----------------------------------------------------------------------------*/
			/** Computes a rectangle that covers a region of points.
			*/
			static void region2rect(struct point * reg, int reg_size,
				const image_double& modgrad, calcFLT reg_angle,
				calcFLT prec, calcFLT p, struct rect * rec)
			{
				calcFLT x, y, dx, dy, l, w, theta, weight, sum, l_min, l_max, w_min, w_max;

#if _DEBUG
				/* check parameters */
				if (reg == nullptr) throw std::invalid_argument("region2rect: invalid region.");
				if (reg_size <= 1) throw std::invalid_argument("region2rect: region size <= 1.");
				if (modgrad.data == NULL)
					throw std::invalid_argument("region2rect: invalid image 'modgrad'.");
				if (rec == nullptr) throw std::invalid_argument("region2rect: invalid 'rec'.");
#endif

				/* center of the region:

				It is computed as the weighted sum of the coordinates
				of all the pixels in the region. The norm of the gradient
				is used as the weight of a pixel. The sum is as follows:
				cx = \sum_i G(i).x_i
				cy = \sum_i G(i).y_i
				where G(i) is the norm of the gradient of pixel i
				and x_i,y_i are its coordinates.
				*/
				x = y = sum = 0;
				for (int i = 0; i < reg_size; i++)
				{
					weight = modgrad.data[reg[i].x + reg[i].y * modgrad.xsize];
					x += (calcFLT)reg[i].x * weight;
					y += (calcFLT)reg[i].y * weight;
					sum += weight;
				}
				if (sum <= 0.0) throw std::invalid_argument("region2rect: weights sum equal to zero.");
				x /= sum;
				y /= sum;

				/* theta */
				theta = get_theta(reg, reg_size, x, y, modgrad, reg_angle, prec);

				/* length and width:

				'l' and 'w' are computed as the distance from the center of the
				region to pixel i, projected along the rectangle axis (dx,dy) and
				to the orthogonal axis (-dy,dx), respectively.

				The length of the rectangle goes from l_min to l_max, where l_min
				and l_max are the minimum and maximum values of l in the region.
				Analogously, the width is selected from w_min to w_max, where
				w_min and w_max are the minimum and maximum of w for the pixels
				in the region.
				*/
				dx = cos(theta);
				dy = sin(theta);
				l_min = l_max = w_min = w_max = 0.0;
				for (int i = 0; i < reg_size; i++)
				{
					l = ((calcFLT)reg[i].x - x) * dx + ((calcFLT)reg[i].y - y) * dy;
					w = -((calcFLT)reg[i].x - x) * dy + ((calcFLT)reg[i].y - y) * dx;

					if (l > l_max) l_max = l;
					if (l < l_min) l_min = l;
					if (w > w_max) w_max = w;
					if (w < w_min) w_min = w;
				}

				/* store values */
				rec->x1 = x + l_min * dx;
				rec->y1 = y + l_min * dy;
				rec->x2 = x + l_max * dx;
				rec->y2 = y + l_max * dy;
				rec->width = w_max - w_min;
				rec->x = x;
				rec->y = y;
				rec->theta = theta;
				rec->dx = dx;
				rec->dy = dy;
				rec->prec = prec;
				rec->p = p;

				/* we impose a minimal width of one pixel

				A sharp horizontal or vertical step would produce a perfectly
				horizontal or vertical region. The width computed would be
				zero. But that corresponds to a one pixels width transition in
				the image.
				*/
				if (rec->width < 1.0) rec->width = 1.0;
			}

			/*----------------------------------------------------------------------------*/
			/** Is point (x,y) aligned to angle theta, up to precision 'prec'?
			*/
			static bool isaligned(int x, int y, const image_double& angles, calcFLT theta, calcFLT prec)
			{
#if _DEBUG
				/* check parameters */
				if (angles.data == NULL)
					throw std::invalid_argument("isaligned: invalid image 'angles'.");
				if (x < 0 || y < 0 || x >= (int)angles.xsize || y >= (int)angles.ysize)
					throw std::invalid_argument("isaligned: (x,y) out of the image.");
				if (prec < 0.0) throw std::invalid_argument("isaligned: 'prec' must be positive.");
#endif

				/* angle at pixel (x,y) */
				calcFLT a = angles.data[x + y * angles.xsize];

				/* pixels whose level-line angle is not defined
				are considered as NON-aligned */
				if (IsNOTDEF(a)/*a == NOTDEF*/) return false;  /* there is no need to call the function
															   'double_equal' here because there is
															   no risk of problems related to the
															   comparison doubles, we are only
															   interested in the exact NOTDEF value */

															   /* it is assumed that 'theta' and 'a' are in the range [-pi,pi] */
															   /** 3/2 pi */
															   //const calcFLT M_3_2_PI = 4.71238898038;

															   /** 2 pi */
															   //const calcFLT M_2__PI = 6.28318530718;


				theta -= a;
				if (theta < 0.0) theta = -theta;
				if (theta > Constants_3_2_PI())
				{
					theta -= Constants_2PI();
					if (theta < 0.0) theta = -theta;
				}

				return theta <= prec;
			}

			static calcFLT get_theta(point * reg, int reg_size, calcFLT x, calcFLT y, const image_double& modgrad, calcFLT reg_angle, calcFLT prec)
			{
				calcFLT Ixx = 0;
				calcFLT Iyy = 0;
				calcFLT Ixy = 0;

#if _DEBUG
				/* check parameters */
				if (reg == nullptr) throw std::invalid_argument("get_theta: invalid region.");
				if (reg_size <= 1) throw std::invalid_argument("get_theta: region size <= 1.");
				if (modgrad.data == nullptr)
					throw std::invalid_argument("get_theta: invalid 'modgrad'.");
				if (prec < 0) throw std::invalid_argument("get_theta: 'prec' must be positive.");
#endif

				/* compute inertia matrix */
				for (int i = 0; i < reg_size; i++)
				{
					calcFLT weight = modgrad.data[reg[i].x + reg[i].y * modgrad.xsize];
					Ixx += ((calcFLT)reg[i].y - y) * ((calcFLT)reg[i].y - y) * weight;
					Iyy += ((calcFLT)reg[i].x - x) * ((calcFLT)reg[i].x - x) * weight;
					Ixy -= ((calcFLT)reg[i].x - x) * ((calcFLT)reg[i].y - y) * weight;
				}
				if (double_equal(Ixx, 0) && double_equal(Iyy, 0) && double_equal(Ixy, 0))
					throw std::invalid_argument("get_theta: null inertia matrix.");

				/* compute smallest eigenvalue */
				calcFLT lambda = (Ixx + Iyy - sqrt((Ixx - Iyy)*(Ixx - Iyy) + 4 * Ixy*Ixy)) / 2;

				/* compute angle */
				calcFLT theta = fabs(Ixx) > fabs(Iyy) ? atan2(lambda - Ixx, Ixy) : atan2(Ixy, lambda - Iyy);

				/* The previous procedure doesn't cares about orientation,
				so it could be wrong by 180 degrees. Here is corrected if necessary. */
				if (angle_diff(theta, reg_angle) > prec) theta += Constants_PI();

				return theta;
			}

			/*----------------------------------------------------------------------------*/
			/** Compare doubles by relative error.

			The resulting rounding error after floating point computations
			depend on the specific operations done. The same number computed by
			different algorithms could present different rounding errors. For a
			useful comparison, an estimation of the relative rounding error
			should be considered and compared to a factor times EPS. The factor
			should be related to the cumulated rounding error in the chain of
			computation. Here, as a simplification, a fixed factor is used.
			*/
			static bool double_equal(calcFLT a, calcFLT b)
			{
				const int RELATIVE_ERROR_FACTOR = 100;

				calcFLT abs_diff, aa, bb, abs_max;

				/* trivial case */
				if (a == b) return true;

				abs_diff = fabs(a - b);
				aa = fabs(a);
				bb = fabs(b);
				abs_max = aa > bb ? aa : bb;

				/* DBL_MIN is the smallest normalized number, thus, the smallest
				number whose relative error is bounded by DBL_EPSILON. For
				smaller numbers, the same quantization steps as for DBL_MIN
				are used. Then, for smaller numbers, a meaningful "relative"
				error should be computed by dividing the difference by DBL_MIN. */
				//if (abs_max < DBL_MIN) abs_max = DBL_MIN;
				if (abs_max < (std::numeric_limits<calcFLT>::min)()) abs_max = (std::numeric_limits<calcFLT>::min)();

				/* equal if relative error <= factor x eps */
				return (abs_diff / abs_max) <= (RELATIVE_ERROR_FACTOR * std::numeric_limits<calcFLT>::epsilon()/*DBL_EPSILON*/);
			}

			static calcFLT angle_diff_signed(calcFLT a, calcFLT b)
			{
				a -= b;
				while (a <= -Constants_PI()) a += Constants_2PI();
				while (a > Constants_PI()) a -= Constants_2PI();
				return a;
			}

			static calcFLT angle_diff(calcFLT a, calcFLT b)
			{
				return std::abs(angle_diff_signed(a, b));
			}

			template <class tSampler>
			static image_double ll_angle(const tSampler in, calcFLT threshold, coorlist ** list_p, void ** mem_p, image_double * modgrad, unsigned int n_bins, LSDNewPerfInfo* perfInfo = nullptr)
			{
				unsigned int  x, y, adr, i;
				calcFLT gx, gy, norm, norm2;
				/* the rest of the variables are used for pseudo-ordering
				the gradient magnitude values */
				int list_count = 0;
				struct coorlist * start;
				struct coorlist * end;
				calcFLT max_grad = 0;

#if _DEBUG
				/* check parameters */
				if (in.XSize() == 0 || in.YSize() == 0) throw std::invalid_argument("ll_angle: invalid image.");
				if (threshold < 0) throw std::invalid_argument("ll_angle: 'threshold' must be positive.");
				if (list_p == nullptr) throw std::invalid_argument("ll_angle: NULL pointer 'list_p'.");
				if (mem_p == nullptr) throw std::invalid_argument("ll_angle: NULL pointer 'mem_p'.");
				if (modgrad == nullptr) throw std::invalid_argument("ll_angle: NULL pointer 'modgrad'.");
				if (n_bins == 0) throw std::invalid_argument("ll_angle: 'n_bins' must be positive.");
#endif

				/* image size shortcuts */
				unsigned int n = in.YSize();
				unsigned int p = in.XSize();

				std::unique_ptr<struct coorlist, std::function<void(coorlist*)>> list((coorlist *)calloc((size_t)(n*p), sizeof(struct coorlist)), [](coorlist* ptr) {free(ptr); });
				std::unique_ptr < coorlist *, std::function<void(coorlist**)>> range_l_s((coorlist**)calloc((size_t)n_bins, sizeof(struct coorlist *)), [](coorlist ** ptr) {free(ptr); });
				std::unique_ptr < coorlist *, std::function<void(coorlist**)>> range_l_e((coorlist**)calloc((size_t)n_bins, sizeof(struct coorlist *)), [](coorlist ** ptr) {free(ptr); });

				if (list == nullptr || !range_l_s || !range_l_e)
					throw std::invalid_argument("not enough memory.");
				for (i = 0; i < n_bins; i++)
				{
					range_l_s.get()[i] = range_l_e.get()[i] = nullptr;
				}

				/* allocate output image */
				image_double g = new_image_double(in.XSize(), in.YSize());

				/* get memory for the image of gradient modulus */
				*modgrad = new_image_double(in.XSize(), in.YSize());

				/* 'undefined' on the down and right boundaries */
				for (x = 0; x < p; x++) { SetToNOTDEF(g.data[(n - 1)*p + x]); }
				for (y = 0; y < n; y++) { SetToNOTDEF(g.data[p*y + p - 1]); }

				tStopWatch sw;
				if (perfInfo != nullptr)
				{
					sw.Initialize();
					sw.startTimer();
				}

				/* compute gradient on the remaining pixels */
				for (x = 0; x < p - 1; x++)
					for (y = 0; y < n - 1; y++)
					{
						adr = y*p + x;

						/*
						Norm 2 computation using 2x2 pixel window:
						A B
						C D
						and
						com1 = D-A,  com2 = B-C.
						Then
						gx = B+D - (A+C)   horizontal difference
						gy = C+D - (A+B)   vertical difference
						com1 and com2 are just to avoid 2 additions.
						*/
						//calcFLT com1 = in.data[adr + p + 1] - in.data[adr];
						//calcFLT com2 = in.data[adr + 1] - in.data[adr + p];

						calcFLT com1, com2;
						in.getCom1Com2(x, y, com1, com2);

						gx = com1 + com2; /* gradient x component */
						gy = com1 - com2; /* gradient y component */
						norm2 = gx*gx + gy*gy;
						norm = sqrt(norm2 / 4); /* gradient norm */

						(*modgrad).data[adr] = norm; /* store gradient norm */

						if (norm <= threshold) /* norm too small, gradient no defined */
							SetToNOTDEF(g.data[adr])/* = NOTDEF*/; /* gradient angle not defined */
						else
						{
							/* gradient angle computation */
							g.data[adr] = atan2(gx, -gy);

							/* look for the maximum of the gradient */
							if (norm > max_grad) max_grad = norm;
						}
					}

				if (perfInfo != nullptr)
				{
					sw.stopTimer();
					perfInfo->time1 = sw.getElapsedTime();
					sw.startTimer();
				}

				/* compute histogram of gradient values */
				for (x = 0; x < p - 1; x++)
					for (y = 0; y < n - 1; y++)
					{
						norm = (*modgrad).data[y*p + x];

						/* store the point in the right bin according to its norm */
						i = static_cast<unsigned int>(norm * n_bins / max_grad);
						if (i >= n_bins) i = n_bins - 1;
						if (range_l_e.get()[i] == nullptr)
							range_l_s.get()[i] = range_l_e.get()[i] = list.get() + list_count++;
						else
						{
							range_l_e.get()[i]->next = list.get() + list_count;
							range_l_e.get()[i] = list.get() + list_count++;
						}
						range_l_e.get()[i]->x = static_cast<int>(x);
						range_l_e.get()[i]->y = static_cast<int>(y);
						range_l_e.get()[i]->next = nullptr;
					}

				if (perfInfo != nullptr)
				{
					sw.stopTimer();
					perfInfo->time2 = sw.getElapsedTime();
					sw.startTimer();
				}

				/* Make the list of pixels (almost) ordered by norm value.
				It starts by the larger bin, so the list starts by the
				pixels with the highest gradient value. Pixels would be ordered
				by norm value, up to a precision given by max_grad/n_bins.
				*/
				for (i = n_bins - 1; i > 0 && range_l_s.get()[i] == nullptr; i--);
				start = range_l_s.get()[i];
				end = range_l_e.get()[i];
				if (start != nullptr)
					while (i > 0)
					{
						--i;
						if (range_l_s.get()[i] != nullptr)
						{
							end->next = range_l_s.get()[i];
							end = range_l_e.get()[i];
						}
					}

				if (perfInfo != nullptr)
				{
					sw.stopTimer();
					perfInfo->time3 = sw.getElapsedTime();
				}

				*list_p = start;
				*mem_p = list.release();

				return g;
			}

			/** Create a new image_double of size 'xsize' times 'ysize'.
			*/
			static image_double new_image_double(unsigned int xsize, unsigned int ysize)
			{
				image_double image;

#if _DEBUG
				/* check parameters */
				if (xsize == 0 || ysize == 0) throw std::invalid_argument("new_image_double: invalid image size.");
#endif

				/* get memory */
				image.data = (imgFLT*)calloc((size_t)(xsize*ysize), sizeof(imgFLT));
				if (image.data == nullptr) throw std::invalid_argument("not enough memory.");

				/* set image size */
				image.xsize = xsize;
				image.ysize = ysize;

				return image;
			}
		};
	}
}