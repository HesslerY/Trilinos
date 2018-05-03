#!/bin/bash -l

if [ "${SRUN_CTEST_TIME_LIMIT_MINUTES}" == "" ] ; then
  SRUN_CTEST_TIME_LIMIT_MINUTES=90
  # This is just running tests, not the entire build!
fi

set -x

source $WORKSPACE/Trilinos/cmake/ctest/drivers/atdm/ctest-s-driver-config-build.sh

set -x

/usr/bin/srun -N 1 --time=${SRUN_CTEST_TIME_LIMIT_MINUTES} --account=fy150090 -J $JOB_NAME \
  $WORKSPACE/Trilinos/cmake/ctest/drivers/atdm/ctest-s-driver-test.sh