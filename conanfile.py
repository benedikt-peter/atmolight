from conans import ConanFile, CMake


class AtmolightConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package", "cmake_paths"
    options = {"opencv": [True, False]}
    default_options = {
        "opencv": False,
        "opencv:jpeg": False,
        "opencv:jpeg2000 ": "jasper",
        "opencv:tiff": False,
        "opencv:webp": False,
        "opencv:png": False,
        "opencv:openexr": False,
        "opencv:dc1394": False,
        "opencv:protobuf": False,
        "opencv:freetype": False,
        "opencv:harfbuzz": False,
        "opencv:eigen": False,
        "opencv:glog": False,
        "opencv:gflags": False,
        "opencv:quirc": False,
        "boost:without_atomic": True,
        "boost:without_chrono": True,
        "boost:without_container": True,
        "boost:without_context": True,
        "boost:without_contract": True,
        "boost:without_coroutine": True,
        "boost:without_date_time": True,
        "boost:without_exception": True,
        "boost:without_fiber": True,
        "boost:without_filesystem": True,
        "boost:without_graph": True,
        "boost:without_graph_parallel": True,
        "boost:without_iostreams": True,
        "boost:without_locale": True,
        "boost:without_log": True,
        "boost:without_math": True,
        "boost:without_mpi": True,
        "boost:without_program_options": True,
        "boost:without_python": True,
        "boost:without_random": True,
        "boost:without_regex": True,
        "boost:without_serialization": True,
        "boost:without_stacktrace": True,
        "boost:without_system": False,
        "boost:without_test": True,
        "boost:without_thread": True,
        "boost:without_timer": True,
        "boost:without_type_erasure": True,
        "boost:without_wave": True
    }

    def requirements(self):
        self.requires("yaml-cpp/0.6.3")
        self.requires("gsl_microsoft/20180102@bincrafters/stable")
        self.requires("CLI11/1.9.1@cliutils/stable")
        self.requires("spdlog/1.7.0")
        self.requires("nlohmann_json/3.9.1")
        self.requires("boost/1.74.0")
        if self.options.opencv:
            self.requires("opencv/4.3.0@conan/stable")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
