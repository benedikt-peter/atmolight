add_executable(atmolight
        atmolight.cpp)

target_link_libraries(atmolight atmodevice atmoanalyzer Boost::Boost nlohmann_json::nlohmann_json fmt::fmt gsl_microsoft::gsl_microsoft yaml-cpp::yaml-cpp CLI11::CLI11 spdlog::spdlog)

install(TARGETS atmolight RUNTIME DESTINATION bin)