# CMakeベースのMakefile
BUILD_DIR = build
PREFIX ?= /usr/local
INSTALL_PREFIX_FILE = $(BUILD_DIR)/.installed_prefix

# 親の make に -j が指定された場合だけ、同じ並列指定を CMake に渡す。
# MAKEFLAGS は -j、-jN、--jobs=N のいずれも通常 -jN の形に正規化する。
MAKE_JOBS = $(patsubst -j%,%,$(filter -j%,$(MAKEFLAGS)))
CMAKE_PARALLEL_ARGS = $(if $(filter -j,$(MAKEFLAGS)),-j,$(if $(MAKE_JOBS),-j$(MAKE_JOBS)))

.PHONY: all clean test install uninstall update debug build debug-build release-build

all: release-build

build: release-build

debug: debug-build
	gdb ./$(BUILD_DIR)/cpt

clean:
	if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR); fi

test: build
	./scripts/test-runner.sh

install: release-build
	cmake --install $(BUILD_DIR) --prefix $(PREFIX)
	@mkdir -p $(BUILD_DIR)
	@echo "$(PREFIX)" > $(INSTALL_PREFIX_FILE)

uninstall:
	BUILD_DIR=$(BUILD_DIR) PREFIX=$(PREFIX) bash ./scripts/make-uninstall.sh

update:
	BUILD_DIR=$(BUILD_DIR) PREFIX=$(PREFIX) bash ./scripts/make-update.sh

# デバッグビルド用
debug-build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) $(CMAKE_PARALLEL_ARGS)

# リリースビルド用
release-build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) $(CMAKE_PARALLEL_ARGS)
