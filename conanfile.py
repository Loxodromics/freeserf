from conan import ConanFile

class FreeSerfConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    
    def requirements(self):
        # Use the lowest common SDL version that all packages support
        self.requires("sdl/2.28.3", override=True)
        self.requires("sdl_mixer/2.8.0") 
        self.requires("sdl_image/2.8.2")
        self.requires("gtest/1.14.0")
    
    def configure(self):
        # Configure all SDL packages as static libraries
        self.options["sdl"].shared = False
        self.options["sdl_mixer"].shared = False
        self.options["sdl_image"].shared = False
        self.options["gtest"].shared = False