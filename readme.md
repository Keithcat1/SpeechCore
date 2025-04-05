# SpeechCore

SpeechCore is a cross-platform C++ library that abstracts the process of communicating with various screen readers, managing low-level details and providing a clean, simple-to-use interface.

## Features

* Simple and intuitive API with straightforward usage
* Cross-platform support for various screen readers, with the option to support additional ones

## Installation

1. Download the matching library build for your platform from [the releases page](https://github.com/still-standing88/SpeechCore/releases).
2. Link against the library (static or shared, depending on your choice).
3. Copy the contents of the include folder to your project directory.
4. Include `SpeechCore.h` in your project.

## Building
### With CMake
You need CMake, get it through chocolatey, scoop, apt or brue depending on your platform. Then git clone the repo and:
```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DSPEECH_CORE_SCREEN_READER_APIS=ON ..
ninja build
```
The SPEECH_CORE_INSTALL_SCREEN_READER_APIS option is optional and causes `ninja install` to copy the Windows DLLs used to interface with screen readers into the bin folder.
There is also `SPEECH_CORE_STATIC_SCREEN_READER_APIS`, which statically links these DLLs for you, currently only NVDA is supported. This means you don't have to coppy them around with your application. This currently requires that you statically link with the C runtime library. CMake should do this automatically if you enable this option, but if you're manually linking it might not and the linker will explode, so compile your .c and .cpp files with /MT.
You can build some examples with `SPEECH_CORE_BUILD_EXAMPLES` option set to ON.

### Manually
* Define either `__SPEECH_C_EXPORT` (shared) or `SPEECH_C_STATIC` (static) when compiling.
* JNI files are included for Java support. Ensure you have the required Java files, or exclude them if not needed.
* Include only platform-specific files for your target platform.
* Linux builds require the Speech Dispatcher library.
* Windows builds need to link against SAPI.LIB.
* Macos builds need to link against object library. And the AVFoundation and Foundation frameworks.
* Documentation generation requires Doxygen and Sphinx.

## Usage

Simple usage example:

```cpp
#include <iostream>
#include <SpeechCore.h>

int main() {
    Speech_Init();
    if (Speech_Is_Loaded()) {
        std::cout << "Current screen reader: " << Speech_Get_Current_Driver() << std::endl;
        std::cout << "Speaking some text" << std::endl;
        Speech_Output(L"This is a test for the SpeechCore library. If you're hearing this, it indicates the library is functioning properly.");
    }

    Speech_Free(); // Free resources when you're done.
    return 0;
}
```

See the documentation for more detailed usage examples.


# Documentation

Documentation can be found [here](https://still-standing88.github.io/SpeechCoreDocs/).

## Library Notes

* Windows screen readers (NVDA, JAWS, Zhengdu, and System Access) require specific binaries, included with the source code.
* Enhanced control over SAPI 5, including voice configuration and speech parameters.
* Braille output functionality is included but not yet implemented.

## Supported Screen Readers

* Windows: NVDA, JAWS, System Access, Zhengdu Screen Reader, SAPI 5
* macOS: AVSpeech
* Linux: Speech Dispatcher

## Bindings

Bindings are available for Python, .NET/C#, and Java.

Install python bindings via pip:
```bash
pip install speech_core
```
available  via the [GitHub repo](https://github.com/still-standing88/Py-SpeechCore).


## Credits

This library was inspired by [Tolk](https://github.com/dkager/tolk/), with adaptations for more flexibility. It was initially developed for personal projects and later expanded to be cross-platform.

## Contributions

Contributions to the library or support for additional screen readers are welcome.

## License

[MIT License](LICENSE)
