from conan import ConanFile

class FreeSerfConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    options = {"use_sdl3": [True, False]}
    default_options = {"use_sdl3": True}
    
    def requirements(self):
        if self.options.use_sdl3:
            # Use SDL3 from Conan
            self.requires("sdl/3.2.14")
            # Note: SDL3_mixer and SDL3_image may not be available yet in Conan
            # They will be built from source via FetchContent if needed
        else:
            # Use the lowest common SDL2 version that all packages support
            self.requires("sdl/2.28.3", override=True)
            self.requires("sdl_mixer/2.8.0") 
            self.requires("sdl_image/2.8.2")
        
        self.requires("gtest/1.14.0")
    
    def configure(self):
        # Configure all SDL packages as static libraries
        self.options["sdl"].shared = False
        if not self.options.use_sdl3:
            # Only configure SDL2 mixer and image packages
            self.options["sdl_mixer"].shared = False
            self.options["sdl_image"].shared = False
        self.options["gtest"].shared = False
