# Get Environment Variable for Samples

This is a helper library for samples that deals with getting environment variables.

Since `getenv()` may generate warnings on MSVC, and is not available on UWP, sample code gets cluttered with minor platform-specific nuances. This library hides all that, so that `std::getenv()` compiles and works the same on Linux, macOS, Win32, and UWP.
