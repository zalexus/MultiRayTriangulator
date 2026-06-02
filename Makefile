# Makefile для сборки плагина MultiRayTriangulator для Nuke 16.0

# Пути
NUKE_ROOT = /cg/soft/nuke/Nuke16.0v8
PLUGIN_DIR = /cg/tank/nuke/plugins/16.0

# Команда сборки (работающая)
CXX = g++
CXXFLAGS = -shared -fPIC -O2 -Wno-deprecated-declarations
INCLUDES = -I$(NUKE_ROOT)/include -DNDK_PROC
LIBS = -L$(NUKE_ROOT) -lDDImage

# Целевые файлы
SOURCES = MultiRayTriangulator.cpp knob_changed.cpp knobs.cpp build_handles.cpp
TARGET = build/MultiRayTriangulator.so

# Цели
.PHONY: all clean build install help

all: clean build install
	@echo "=== Build and install complete ==="

build:
	@echo "=== Creating build directory ==="
	@mkdir -p build
	@echo "=== Compiling and linking ==="
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) $(LIBS) -o $(TARGET)
	@echo "=== Build complete ==="
	@ls -la $(TARGET)

install: build
	@echo "=== Installing to $(PLUGIN_DIR) ==="
	@mkdir -p $(PLUGIN_DIR)
# 	rm -f $(PLUGIN_DIR)/MultiRayTriangulator.so
	cp $(TARGET) $(PLUGIN_DIR)/
	@echo "=== Installed: $(PLUGIN_DIR)/MultiRayTriangulator.so ==="
	@ls -la $(PLUGIN_DIR)/MultiRayTriangulator.so

clean:
	@echo "=== Cleaning build directory ==="
	@rm -rf build
	@rm -f $(PLUGIN_DIR)/MultiRayTriangulator.so
	@echo "=== Clean complete ==="

help:
	@echo "MultiRayTriangulator Nuke Plugin Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make clean    - Remove build directory"
	@echo "  make build    - Compile the plugin"
	@echo "  make install  - Copy plugin to Nuke plugins directory"
	@echo "  make all      - clean + build + install"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Build command:"
	@echo "  $(CXX) $(CXXFLAGS) $(INCLUDES) $(SOURCES) $(LIBS) -o $(TARGET)"

# CMake сборка
cmake: clean
	@echo "=== Building with CMake ==="
	@./cmake_build.sh

cmake-install: cmake
	@echo "=== Already installed during build ==="

cmake-clean:
	@echo "=== Cleaning CMake build directory ==="
	@rm -rf cmake_build

.PHONY: cmake cmake-install cmake-clean
