PROG            = ifcomp

all:            build
		$(MAKE) -C build

build:
		cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug $(CMAKE_ARGS)

install:        all
		cmake --install build

test:           all
		ctest --test-dir build/tests

clean:
		rm -rf build ifcomp_test_[ab]_* ifcomp_test_out_*

reindent:
		@echo "Running clang-format on C++ sources..."
		@command -v clang-format >/dev/null 2>&1 || { echo "Error: clang-format not found in PATH"; exit 1; }
		@clang-format -i *.h *.cpp tests/*.h tests/*.cpp
