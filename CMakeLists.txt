cmake_minimum_required(VERSION 3.15)

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

option(WITH_JAVA "Enable Java support" OFF)
option(SPEECH_CORE_INSTALL_SCREEN_READER_APIS "Install screen reader API dynamic libraries (currently windows only)" ON)
option(SPEECH_CORE_STATIC_SCREEN_READER_APIS "Statically link to screen reader API libraries (currently windows only)" OFF)


# find bin and lib paths
set(STATIC_LIB_ARCH_PATH "")
set(DYNAMIC_LIB_ARCH_PATH "")

# First determine bitness using CMAKE_SIZEOF_VOID_P which works correctly
# even when cross-compiling 32-bit on 64-bit Windows
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64-bit build - now determine if it's x64 or ARM64
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR 
	(MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64"))
		set(DYNAMIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/arm64")
		set(STATIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/arm64")
	else()
		# Default 64-bit is x64
		set(DYNAMIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/x64")
		set(STATIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/x64")
	endif()
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	# 32-bit build is x86
	set(DYNAMIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/x86")
	set(STATIC_LIB_ARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/x86")
else()
	message(ERROR "Unknown pointer size: ${CMAKE_SIZEOF_VOID_P}. Library path not set correctly.")
endif()

if(NOT EXISTS "${DYNAMIC_LIB_ARCH_PATH}")
    message(ERROR "Library directory '${DYNAMIC_LIB_ARCH_PATH}' does not exist!")
endif()

if(NOT EXISTS "${STATIC_LIB_ARCH_PATH}")
    message(ERROR "Library directory '${STATIC_LIB_ARCH_PATH}' does not exist!")
endif()








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
    target_compile_definitions(SpeechCore PUBLIC __SPEECH_C_EXPORT)
    if(NOT WIN32 AND (PLATFORM STREQUAL "linux" OR PLATFORM STREQUAL "macos"))
        target_compile_options(SpeechCore PRIVATE -fPIC)
    endif()
else()
    target_compile_definitions(SpeechCore PUBLIC __SPEECH_C_STATIC)
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
	if(SPEECH_CORE_STATIC_SCREEN_READER_APIS)
	set_target_properties(ExampleC PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
	set_target_properties(ExampleC++ PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")

endif()
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

if(WIN32 AND SPEECH_CORE_STATIC_SCREEN_READER_APIS)
	target_compile_definitions(SpeechCore PRIVATE SPEECH_CORE_STATIC_SCREEN_READER_APIS)
	add_library(NvdaControllerClientStatic STATIC IMPORTED)
	set_target_properties(NvdaControllerClientStatic PROPERTIES IMPORTED_LOCATION "${STATIC_LIB_ARCH_PATH}/nvdaControllerClient_static.lib")
	target_link_libraries(SpeechCore PRIVATE NvdaControllerClientStatic rpcrt4)
	set_target_properties(SpeechCore PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

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
	if(SPEECH_CORE_STATIC_SCREEN_READER_APIS)
    file(GLOB DLL_FILES "${STATIC_LIB_ARCH_PATH}/*_static.lib")
    
    
    install(
      FILES ${DLL_FILES}
      DESTINATION lib
      COMPONENT Development
    )
else()
		file(GLOB DLL_FILES "${DYNAMIC_LIB_ARCH_PATH}/*.dll")
		# Install DLLs to the bin directory
		install(
			FILES ${DLL_FILES}
			DESTINATION bin
			COMPONENT Runtime
		)
	endif()
endif()