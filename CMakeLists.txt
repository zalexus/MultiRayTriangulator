cmake_minimum_required(VERSION 3.10)
# ИСПРАВЛЕНО: Изменено имя проекта на MultiRayTriangulator
project(MultiRayTriangulator)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Путь к Nuke (можно переопределить через -DNUKE_ROOT=...)
set(NUKE_ROOT "/cg/soft/nuke/Nuke16.0v8" CACHE PATH "Path to Nuke installation")

# Директория для установки плагина
set(PLUGIN_INSTALL_DIR "/cg/tank/nuke/plugins/16.0" CACHE PATH "Nuke plugins directory")

# ИСПРАВЛЕНО: Находим системную библиотеку OpenGL для отрисовки красного крестика
find_package(OpenGL REQUIRED)

# ИСПРАВЛЕНО: Обновлен список SOURCES — добавлены два раздельных .cpp файла
set(SOURCES
    MultiRayTriangulator.cpp
    knobs.cpp
    knob_changed.cpp
)

# ИСПРАВЛЕНО: Создаём общую библиотеку с новым именем MultiRayTriangulator
add_library(MultiRayTriangulator SHARED ${SOURCES})

# Убираем префикс "lib" и оставляем только название библиотеки
set_target_properties(MultiRayTriangulator PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# Флаги компиляции
target_compile_options(MultiRayTriangulator PRIVATE
    -fPIC
    -O2
    -Wno-deprecated-declarations
)

# Определения препроцессора
target_compile_definitions(MultiRayTriangulator PRIVATE
    NUKEDK_BUILD
    NDK_PROC
)

# Пути для заголовочных файлов
target_include_directories(MultiRayTriangulator PRIVATE
    ${NUKE_ROOT}/include
    ${OPENGL_INCLUDE_DIR}
)

# Пути для библиотек и линковка
target_link_directories(MultiRayTriangulator PRIVATE
    ${NUKE_ROOT}
)

# ИСПРАВЛЕНО: Линкуем DDImage и найденный через CMake таргет OpenGL
target_link_libraries(MultiRayTriangulator
    DDImage
    ${OPENGL_LIBRARIES}
)

# Установка плагина в директорию Nuke
install(TARGETS MultiRayTriangulator
    LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}
)

# Вывод информации
message(STATUS "NUKE_ROOT: ${NUKE_ROOT}")
message(STATUS "PLUGIN_INSTALL_DIR: ${PLUGIN_INSTALL_DIR}")
