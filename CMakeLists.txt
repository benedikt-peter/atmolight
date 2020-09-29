cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)

project(atmolight)

### Compiler/Linker settings

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    message(SEND_ERROR "Intel C++ Compiler is not supported!")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/bigobj)
endif ()

# Conan

if (EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
else ()
    message(WARNING "The file conan_paths.cmake doesn't exist. You have to run conan install first or provide the dependencies by yourself!")
endif ()

# Dependencies

find_package(fmt REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)
find_package(gsl_microsoft REQUIRED)
find_package(CLI11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(opencv QUIET)
find_package(Boost 1.74.0 COMPONENTS system test REQUIRED)

# Optional dependencies

if(TARGET opencv::opencv)
    message("Found OpenCV")
    add_compile_definitions(WITH_OPENCV)
else()
    message("OpenCV support disabled")
endif()

check_include_file(linux/spi/spidev.h WITH_SPI)
if (WITH_SPI)
    message("Found SPI")
    add_compile_definitions(WITH_SPI)
else()
    message("SPI device support disabled")
endif()

# Include libraries and executables

add_subdirectory(src)

# Include tests

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Config files installation

install(FILES util/systemd/atmolight.service DESTINATION /etc/systemd/system/)
install(FILES config/example.yml DESTINATION /etc/atmolight/)