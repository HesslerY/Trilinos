TRIBITS_ADD_EXECUTABLE(
  hts_test NOEXEPREFIX
  SOURCES hts_test.cpp)

TRIBITS_ADD_ADVANCED_TEST(
  hts_test_1
  TEST_0 EXEC hts_test NOEXEPREFIX
    NUM_MPI_PROCS 1
    NUM_TOTAL_CORES_USED 2
    PASS_REGULAR_EXPRESSION Passed
  COMM serial mpi
  ENVIRONMENT OMP_NUM_THREADS=2)
