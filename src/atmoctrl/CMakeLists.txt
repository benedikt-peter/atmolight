add_executable(atmoctrl
        atmoctrl.cpp)

target_link_libraries(atmoctrl atmodevice CLI11::CLI11 fmt::fmt)

install(TARGETS atmoctrl RUNTIME DESTINATION bin)