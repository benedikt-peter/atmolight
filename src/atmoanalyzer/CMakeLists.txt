add_library(atmoanalyzer STATIC
        config.cpp
        open_cv_capture.cpp
        analyzer.cpp
        control_server.cpp
        devices.cpp
        request_handler.cpp
        application.cpp)

target_include_directories(atmoanalyzer PUBLIC include)

target_link_libraries(atmoanalyzer atmodevice Boost::Boost nlohmann_json::nlohmann_json fmt::fmt gsl_microsoft::gsl_microsoft yaml-cpp::yaml-cpp CLI11::CLI11 spdlog::spdlog)

if(TARGET opencv::opencv)
    target_link_libraries(atmoanalyzer opencv::opencv)
endif()