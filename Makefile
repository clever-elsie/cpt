# CMakeベースのMakefile
BUILD_DIR = build

.PHONY: all clean test install debug build debug-build release-build

all: build

build:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR)

debug: build
	gdb ./$(BUILD_DIR)/cpt

clean:
	if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR); fi

test: build
	./scripts/test-runner.sh

install: build
	cmake --install $(BUILD_DIR)

# デバッグビルド用
debug-build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR)

# リリースビルド用
release-build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR)
