

set(TESTS
	sequence_buffer_test.cpp
)

add_executable(dna_test ${TESTS} main.cpp)
target_include_directories(dna_test PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
