from conans import ConanFile, CMake, tools


class LzstringcppConan(ConanFile):
    name = "EasyScreenCapturer"
    version = "1.0.2"
    license = "MIT"
    author = "xl"
    url = "https://github.com/243286065/EasyScreenCapturer"
    description = "A lib to capture screen on windows or linux."
    topics = ("Screen capture")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"

    def configure(self):
        if self.settings.os == "Linux":
            self.settings.compiler.libcxx="libstdc++11"

    def source(self):
        self.run("git clone https://github.com/243286065/EasyScreenCapturer.git")

    def build(self):
        cmake = CMake(self)
        cmake.definitions['NO_BUILD_DEMO'] = True
        cmake.configure(source_folder="EasyScreenCapturer")
        cmake.build()

    def package(self):
        self.copy("EasyScreenCapturer.h", dst="include/EasyScreenCapturer", src="EasyScreenCapturer/src")
        self.copy("CaptureStatusCode.h", dst="include/EasyScreenCapturer", src="EasyScreenCapturer/src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["EasyScreenCapturer"]