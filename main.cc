#include <cstdio>
#include <cstdlib>

#include "version.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef WITH_MPI
#include <mpi.h>
#endif

#include "picojson.h"
#include "mmm_io.h"
#include "timerutil.h"

int
main(
  int argc,
  char **argv)
{
  printf("[Mallie] version: %s\n", MALLIE_VERSION);

  mallie::timerutil t;
  t.start();
  printf("[Mallie] Begin\n");
  t.end();
  printf("[Mallie] End\n");
  printf("[Mallie] Elapsed: %d sec(s)\n", (int)t.sec());
  return EXIT_SUCCESS;
}
 
