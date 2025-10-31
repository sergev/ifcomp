PROG            = ifcomp
CFLAGS		= -g -O3 -Wall -Werror
LDFLAGS         = -g
LIBCMOCKA       = -lcmocka

ifneq ($(wildcard /usr/local/include),)
CFLAGS		+= -I/usr/local/include
endif
ifneq ($(wildcard /opt/homebrew/include),)
CFLAGS		+= -I/opt/homebrew/include
endif
ifneq ($(wildcard /usr/local/lib),)
LIBCMOCKA       += -L/usr/local/lib
endif
ifneq ($(wildcard /opt/homebrew/lib),)
LIBCMOCKA       += -L/opt/homebrew/lib
endif

all:            build
		$(MAKE) -C build

build:
		cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug $(CMAKE_ARGS)

install:        all
		cmake --install build

test:           all
		ctest --test-dir build/tests

clean:
		rm -rf build

reindent:
		@echo "Running clang-format on C++ sources..."
		@command -v clang-format >/dev/null 2>&1 || { echo "Error: clang-format not found in PATH"; exit 1; }
		@clang-format -i *.h *.c tests/*.cpp
