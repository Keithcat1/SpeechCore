cmake_minimum_required(VERSION 3.14)

# Project configuration
project(SpeechCore 
        VERSION 1.0.0
        DESCRIPTION "SpeechCore Library"
        LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(SPEECH_CORE_BUILD_EXAMPLES "Build the C and C++ examples" OFF)

option(WITH_JAVA "Enable Java support" ON)
option(SPEECH_CORE_INSTALL_SCREEN_READER_APIS "Install screen reader API dynamic libraries (currently windows only)" ON)

# Architecture detection
include(CheckCXXCompilerFlag)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
    set(ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686")
    set(ARCH "x86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(ARCH "arm64")
else()
    message(WARNING "Unrecognized architecture '${CMAKE_SYSTEM_PROCESSOR}'. Defaulting to x86_64.")
    set(ARCH "x86_64")
endif()

# Platform detection
if(WIN32)
    set(PLATFORM "windows")
elseif(APPLE)
    set(PLATFORM "macos")
else()
    set(PLATFORM "linux")
endif()

# Compiler detection
if(MSVC)
    set(COMPILER "msvc")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(COMPILER "clang")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(COMPILER "gcc")
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
    set(COMPILER "unknown")
endif()

message(STATUS "Detected platform: ${PLATFORM}")
message(STATUS "Building for architecture: ${ARCH}")
message(STATUS "Using compiler: ${COMPILER}")





# Create library target
add_library(SpeechCore "")

# Set include directory
target_include_directories(SpeechCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Common definitions
target_compile_definitions(SpeechCore PRIVATE UNICODE _UNICODE)



# Handle build mode specific flags
if(BUILD_SHARED_LIBS)
    target_compile_definitions(SpeechCore PRIVATE __SPEECH_C_EXPORT)
    if(NOT WIN32 AND (PLATFORM STREQUAL "linux" OR PLATFORM STREQUAL "macos"))
        target_compile_options(SpeechCore PRIVATE -fPIC)
    endif()
else()
    target_compile_definitions(SpeechCore PRIVATE __SPEECH_C_STATIC)
endif()

# Compiler-specific options
if(COMPILER STREQUAL "msvc")
    target_compile_options(SpeechCore PRIVATE /EHsc)
    target_compile_definitions(SpeechCore PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(COMPILER STREQUAL "clang")
    target_compile_options(SpeechCore PRIVATE 
        -Wall -Wextra 
        -Wno-extern-initializer -Wno-unused-parameter -Wno-deprecated-declarations)
    
    if(APPLE)
        target_compile_options(SpeechCore PRIVATE 
            -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
    endif()
elseif(COMPILER STREQUAL "gcc")
    target_compile_options(SpeechCore PRIVATE 
        -Wall -Wextra -Wno-extern-initializer -Wno-unused-parameter)
endif()

# Platform-specific setup
if(PLATFORM STREQUAL "windows")
    # Windows setup
    if(ARCH STREQUAL "x86_64")
        target_compile_definitions(SpeechCore PRIVATE _WIN64)
    elseif(ARCH STREQUAL "x86")
        target_link_options(SpeechCore PRIVATE /SAFESEH:NO)
        target_compile_definitions(SpeechCore PRIVATE WIN32)
    endif()
 

    target_link_libraries(SpeechCore PRIVATE sapi ole32 user32 oleaut32)
    


    # Windows resources
    if(BUILD_SHARED_LIBS)
        set(RC_FILES "src/SpeechCore.rc")
        set_source_files_properties(${RC_FILES} PROPERTIES LANGUAGE RC)
        target_sources(SpeechCore PRIVATE ${RC_FILES})
    endif()

elseif(PLATFORM STREQUAL "macos")
    # macOS setup
    target_compile_definitions(SpeechCore PRIVATE MACOS)
    target_link_libraries(SpeechCore PRIVATE "-framework Foundation" "-framework AVFoundation" objc)
    set_property(TARGET SpeechCore PROPERTY XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
    
    # Set macOS architectures and minimum version
    target_compile_options(SpeechCore PRIVATE -mmacosx-version-min=10.14)

    # Add Objective-C/C++ compile rules
    set_property(TARGET SpeechCore PROPERTY OBJCXX_EXTENSIONS ON)
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/AVSpeech.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )

elseif(PLATFORM STREQUAL "linux")
    # Linux setup
    target_compile_definitions(SpeechCore PRIVATE LINUX)
    target_link_libraries(SpeechCore PRIVATE speechd)
    target_compile_options(SpeechCore PRIVATE -fPIC)
    
    # Architecture-specific flags
    if(ARCH STREQUAL "arm64")
        target_compile_options(SpeechCore PRIVATE -march=armv8-a)
    endif()
endif()

# Java support
if(WITH_JAVA)
    find_package(Java COMPONENTS Development)
    find_package(JNI)
    
    if(Java_FOUND AND JNI_FOUND)
        message(STATUS "Java support enabled")
        target_compile_definitions(SpeechCore PRIVATE __WITH_JAVA)
        target_include_directories(SpeechCore PRIVATE ${JNI_INCLUDE_DIRS})
        target_link_libraries(SpeechCore PRIVATE ${JNI_LIBRARIES})
        target_sources(SpeechCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/SpeechCore_JNI.cpp)
    else()
        message(STATUS "Java not found. Disabling Java support.")
        set(WITH_JAVA OFF)
    endif()
endif()

if(SPEECH_CORE_BUILD_EXAMPLES)
add_executable(ExampleC
	"examples/c/example.c"
)
target_link_libraries(ExampleC SpeechCore)
add_executable(ExampleC++
	"examples/c++/example.cpp"
)
target_link_libraries(ExampleC++ SpeechCore)
endif()

# Platform-specific file exclusions
set(WINDOWS_EXCLUDE
    sapi5driver.cpp
    SapiSpeech.cpp
    nvda.cpp
    jaws.cpp
    sa.cpp
    zdsr.cpp
    zdsrapi.cpp
    saapi.cpp
    fsapi.c
    wasapi.cpp
)

# Set up source files with platform exclusions
file(GLOB_RECURSE ALL_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm
)

# Filter out files based on platform
set(SOURCES)
foreach(SOURCE ${ALL_SOURCES})
    get_filename_component(FILENAME ${SOURCE} NAME)
    
    set(EXCLUDE FALSE)
    
    # Common exclusions - SpeechCore_JNI.cpp is handled separately
    if(FILENAME STREQUAL "SpeechCore_JNI.cpp")
        set(EXCLUDE TRUE)
    endif()
    
    # Platform-specific exclusions
    if(PLATFORM STREQUAL "windows")
        if(FILENAME STREQUAL "SpeechDispatcher.cpp" OR FILENAME STREQUAL "AVSpeech.mm" OR FILENAME STREQUAL "AVTts.cpp")
            set(EXCLUDE TRUE)
        endif()
    elseif(PLATFORM STREQUAL "macos")
        if(FILENAME STREQUAL "SpeechDispatcher.cpp" OR
           FILENAME IN_LIST WINDOWS_EXCLUDE)
            set(EXCLUDE TRUE)
        endif()
    elseif(PLATFORM STREQUAL "linux")
        if(FILENAME STREQUAL "AVTts.cpp" OR FILENAME STREQUAL "AVSpeech.mm" OR FILENAME IN_LIST WINDOWS_EXCLUDE)
            set(EXCLUDE TRUE)
        endif()
    endif()
    
    if(NOT EXCLUDE)
        list(APPEND SOURCES ${SOURCE})
    endif()
endforeach()

# Add sources to the library target
target_sources(SpeechCore PRIVATE ${SOURCES})


# Installation rules
include(GNUInstallDirs)
install(TARGETS SpeechCore
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# NVDAControllerClient ETC
if(WIN32 AND SPEECH_CORE_INSTALL_SCREEN_READER_APIS)
  # Check architecture to determine which DLL path to use
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit architecture
    set(DLL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/x64")
  else()
    # 32-bit architecture
    set(DLL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/x86")
  endif()

  # Check if the DLL directory exists
  if(EXISTS "${DLL_SOURCE_DIR}")
    # Get all DLLs
    file(GLOB DLL_FILES "${DLL_SOURCE_DIR}/*.dll")
    
    # Install DLLs to the bin directory
    install(
      FILES ${DLL_FILES}
      DESTINATION bin
      COMPONENT Runtime
    )
  else()
    message(WARNING "DLL directory not found: ${DLL_SOURCE_DIR}")
  endif()
endif()