# Get Environment Variable for Samples

This is a helper library for samples that deals with getting environment variables.

Since `std::getenv()` may generate warnings on MSVC, and is not available on UWP, sample code gets cluttered with minor platform-specific nuances. `GetEnv()` function, provided in this library, hides all that.
