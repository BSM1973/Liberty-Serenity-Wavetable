// Only this translation unit is compiled with /arch:AVX2.
#include <immintrin.h>
namespace serenity {
void hdAvx2Interpolate8(const float* a,const float* b,const int* i0,const int* i1,
                       const float* fractions,float blend,float* out) noexcept {
 const auto x0=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(i0));
 const auto x1=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(i1));
 const auto t=_mm256_loadu_ps(fractions);
 const auto a0=_mm256_i32gather_ps(a,x0,4),a1=_mm256_i32gather_ps(a,x1,4);
 const auto x=_mm256_add_ps(a0,_mm256_mul_ps(_mm256_sub_ps(a1,a0),t));
 if(blend<=0.f){_mm256_storeu_ps(out,x);return;}
 const auto b0=_mm256_i32gather_ps(b,x0,4),b1=_mm256_i32gather_ps(b,x1,4);
 const auto y=_mm256_add_ps(b0,_mm256_mul_ps(_mm256_sub_ps(b1,b0),t));
 _mm256_storeu_ps(out,_mm256_add_ps(x,_mm256_mul_ps(_mm256_sub_ps(y,x),_mm256_set1_ps(blend))));
}
}
