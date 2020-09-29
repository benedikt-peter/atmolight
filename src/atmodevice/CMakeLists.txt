add_library(atmodevice STATIC
        device.cpp
        serial_devices.cpp
        spi_devices.cpp)

target_include_directories(atmodevice PUBLIC include)

target_link_libraries(atmodevice Boost::Boost fmt::fmt gsl_microsoft::gsl_microsoft spdlog::spdlog)