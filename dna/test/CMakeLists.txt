

set(TESTS
		fake_stream.cpp
		fake_stream_test.cpp
		sequence_buffer_test.cpp
)

add_executable(dna_test ${TESTS} main.cpp)
target_link_libraries(dna_test cogdna)
