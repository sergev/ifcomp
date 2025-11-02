PROG            = ifcomp

all:            build
		$(MAKE) -C build

build:
		cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug $(CMAKE_ARGS)

install:        all
		cmake --install build

test:           all
		ctest --test-dir build/tests

coverage:       clean
		@echo "Building with coverage enabled..."
		cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
		$(MAKE) -C build
		@echo "Running tests to generate coverage data..."
		ctest --test-dir build/tests
		@echo "Generating coverage report..."
		@if command -v lcov >/dev/null 2>&1; then \
			lcov --capture --directory build --output-file build/coverage.info --exclude '/usr/*' --exclude '*/tests/*' --exclude '*/googletest/*' || true; \
			lcov --remove build/coverage.info '/usr/*' '*/tests/*' '*/googletest/*' --output-file build/coverage.info || true; \
			genhtml build/coverage.info --output-directory build/coverage_html || true; \
			echo "Coverage report generated at build/coverage_html/index.html"; \
			lcov --summary build/coverage.info 2>/dev/null | grep "lines.*:" | head -1 || echo "Coverage summary unavailable"; \
		else \
			echo "lcov not found. Install with: brew install lcov (Mac) or apt-get install lcov (Ubuntu)"; \
			echo "Falling back to gcov..."; \
			gcov -o build/CMakeFiles/compare.dir build/CMakeFiles/compare.dir/*.gcda >/dev/null 2>&1 || true; \
			echo "Calculating coverage percentage..."; \
			python3 -c " \
import re, os; \
total, executed = 0, 0; \
for f in ['ifcomp.cpp', 'pass1.cpp', 'pass2.cpp', 'pass3.cpp', 'pass4.cpp', 'pass5.cpp', 'pass6.cpp', 'pass7.cpp', 'pass8.cpp']: \
	gcov_file = f'{f}.gcov'; \
	if os.path.exists(gcov_file): \
		with open(gcov_file) as g: \
			for line in g: \
				m = re.match(r'^\s*(\d+|#####):\s*\d+:\s*', line); \
				if m and m.group(1).isdigit() and int(m.group(1)) > 0: \
					executed += 1; \
				if m: total += 1; \
print(f'C++ Test Coverage: {(executed/total*100):.1f}%' if total > 0 else 'Coverage data not available')" || \
			echo "Coverage files generated. Check .gcov files for details."; \
		fi

clean:
		rm -rf build ifcomp_test_[ab]_* ifcomp_test_out_* *.gcov

reindent:
		@echo "Running clang-format on C++ sources..."
		@command -v clang-format >/dev/null 2>&1 || { echo "Error: clang-format not found in PATH"; exit 1; }
		@clang-format -i *.h *.cpp tests/*.h tests/*.cpp
