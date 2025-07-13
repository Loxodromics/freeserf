from conan import ConanFile

class FreeSerfConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    options = {"use_sdl3": [True, False]}
    default_options = {"use_sdl3": False}
    
    def requirements(self):
        if self.options.use_sdl3:
            # SDL3 packages (when available in Conan)
            # Note: SDL3 packages may not be available in Conan yet
            # This is prepared for when they become available
            self.requires("sdl/3.0.0", override=True)
            self.requires("sdl_mixer/3.0.0") 
            self.requires("sdl_image/3.0.0")
        else:
            # Use the lowest common SDL2 version that all packages support
            self.requires("sdl/2.28.3", override=True)
            self.requires("sdl_mixer/2.8.0") 
            self.requires("sdl_image/2.8.2")
        
        self.requires("gtest/1.14.0")
    
    def configure(self):
        # Configure all SDL packages as static libraries
        self.options["sdl"].shared = False
        if self.options.use_sdl3:
            self.options["sdl_mixer"].shared = False
            self.options["sdl_image"].shared = False
        else:
            self.options["sdl_mixer"].shared = False
            self.options["sdl_image"].shared = False
        self.options["gtest"].shared = False