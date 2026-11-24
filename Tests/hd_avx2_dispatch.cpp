// Baseline x64 translation unit. Never execute AVX2 before CPU+OS checks.
#include <intrin.h>
namespace serenity {
bool hdAvx2Available() noexcept {
 static const bool available=[]() noexcept {
  int r[4]{};__cpuid(r,0);if(r[0]<7)return false;
  __cpuidex(r,1,0);
  constexpr int mask=(1<<27)|(1<<28);
  if((r[2]&mask)!=mask||(_xgetbv(0)&6)!=6)return false;
  __cpuidex(r,7,0);return (r[1]&(1<<5))!=0;
 }();
 return available;
}
}
