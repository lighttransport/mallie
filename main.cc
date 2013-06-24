#include <cstdio>
#include <cstdlib>

#include "version.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef WITH_MPI
#include <mpi.h>
#endif

#include "mmm_io.h"

int
main(
  int argc,
  char **argv)
{
  printf("[Mallie] version: %s\n", MALLIE_VERSION);

  printf("[Mallie] Begin\n");
  printf("[Mallie] End\n");
  return EXIT_SUCCESS;
}
 
