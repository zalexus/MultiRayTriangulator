#!/bin/bash
# Скрипт сборки через CMake

cd "$(dirname "$0")"

# Параметры
NUKE_ROOT="/cg/soft/nuke/Nuke16.0v8"
PLUGIN_DIR="/cg/tank/nuke/plugins/16.0"

# Удаляем старую сборку
rm -rf cmake_build

# Создаём директорию для сборки
mkdir -p cmake_build
cd cmake_build

# Конфигурация CMake
cmake .. \
    -DNUKE_ROOT=${NUKE_ROOT} \
    -DPLUGIN_INSTALL_DIR=${PLUGIN_DIR}

# Сборка
make -j$(nproc)

# Установка
make install

echo "=== Build complete ==="
# ИСПРАВЛЕНО: Проверяем наличие файла MultiRayTriangulator.so вместо старого названия
ls -la ${PLUGIN_DIR}/MultiRayTriangulator.so
