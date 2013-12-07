#include "gtest/gtest.h"
#include <stdint.h>

#if defined(__sparc__)

#define ASI_P "0x80"

static inline int SparcCompareAndSwapU64(volatile int64_t *addr, int64_t oldval,
                                         int64_t newval) {
  // From OpenMPI.

  /* casa [reg(rs1)] %asi, reg(rs2), reg(rd)
   *
   * if (*(reg(rs1)) == reg(rs1) )
   *    swap reg(rd), *(reg(rs1))
   * else
   *    reg(rd) = *(reg(rs1))
   */
  int64_t ret = newval;

  __asm__ __volatile__("casxa [%1] " ASI_P ", %2, %0"
                       : "+r"(ret)
                       : "r"(addr), "r"(oldval));
  return (ret == oldval);
}

TEST(AtomicTest, Sparc_CAS64) {

  double a = 0.1;
  double b = 0.2;
  double c = 0.3;

  void *addr = &a;
  void *oldaddr = &b;
  void *newaddr = &c;
  int ret = SparcCompareAndSwapU64((int64_t *)&addr,
                                   reinterpret_cast<uintptr_t>(oldaddr),
                                   reinterpret_cast<uintptr_t>(newaddr));

  EXPECT_EQ(ret, 0);

  addr = &a;
  oldaddr = &a;
  newaddr = &c;
  ret = SparcCompareAndSwapU64((int64_t *)&addr,
                               reinterpret_cast<uintptr_t>(oldaddr),
                               reinterpret_cast<uintptr_t>(newaddr));

  EXPECT_EQ(ret, 1);

}
#endif

#if defined(__x86_64__)
TEST(AtomicTest, x86_CAS64) {

  double a = 0.1;
  double b = 0.2;
  double c = 0.3;

  void *addr = &a;
  void *oldaddr = &b;
  void *newaddr = &c;
  printf("addr(before) = %p\n", addr);
  void *ret =
      __sync_val_compare_and_swap(&addr, reinterpret_cast<uintptr_t>(oldaddr),
                                  reinterpret_cast<uintptr_t>(newaddr));
  printf("addr(after) = %p\n", addr);

  EXPECT_EQ(ret, addr);
  EXPECT_EQ(addr, &a);

  addr = &a;
  oldaddr = &a;
  newaddr = &c;
  printf("addr(before) = %p\n", addr);
  ret = __sync_val_compare_and_swap(&addr, reinterpret_cast<uintptr_t>(oldaddr),
                                    reinterpret_cast<uintptr_t>(newaddr));
  printf("addr(after) = %p\n", addr);
  printf("ret = %p\n", ret);
  EXPECT_NE(ret, addr);
  EXPECT_EQ(ret, oldaddr);
  EXPECT_EQ(addr, newaddr);
}
#endif
