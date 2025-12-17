cmake_minimum_required(VERSION 3.22)
project(encdec VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)

list(APPEND encdec_src src/BufferedWriter.h)
list(APPEND encdec_src src/FileReader.h)
list(APPEND encdec_src src/FileWriter.h)
list(APPEND encdec_src src/IDecoder.h)
list(APPEND encdec_src src/IDeserializable.h)
list(APPEND encdec_src src/IEncoder.h)
list(APPEND encdec_src src/IReader.h)
list(APPEND encdec_src src/ISerializable.h)
list(APPEND encdec_src src/IWriter.h)
list(APPEND encdec_src src/JSONParser.cpp src/JSONParser.h)
list(APPEND encdec_src src/JSONEncoder.cpp src/JSONEncoder.h)
list(APPEND encdec_src src/PrintfAttribute.h)
list(APPEND encdec_src src/StringReader.h)
list(APPEND encdec_src src/StringWriter.cpp src/StringWriter.h)
list(APPEND encdec_src src/XMLDecoder.cpp src/XMLDecoder.h)
list(APPEND encdec_src src/XMLEncoder.cpp src/XMLEncoder.h)
list(APPEND encdec_src src/XMLParser.cpp src/XMLParser.h)

list(APPEND encdec_tst_src tests/test_filewriter.cpp)
list(APPEND encdec_tst_src tests/test_jsonencoder.cpp)
list(APPEND encdec_tst_src tests/test_jsonparser.cpp)
list(APPEND encdec_tst_src tests/test_stringreader.cpp)
list(APPEND encdec_tst_src tests/test_xmldecoder.cpp)
list(APPEND encdec_tst_src tests/test_xmlencoder.cpp)

#add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_library(tst_${PROJECT_NAME} SHARED)
add_library(tst_${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "Debug build detected")
    # add_definitions(-DDEBUG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DDEBUG)
    target_compile_definitions(tst_${PROJECT_NAME} PUBLIC -DDEBUG)
endif()

if (WIN32)
elseif(APPLE)
    message(STATUS "OS is macOS")

elseif(UNIX)
    message(STATUS "OS is Linux")
    message(STATUS "CMAKE_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
    target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra -Wshadow -fPIC)
    target_compile_options(tst_${PROJECT_NAME} INTERFACE -Wall -Wextra -Wshadow -fPIC)
    target_compile_definitions(${PROJECT_NAME} INTERFACE -DLINUX)
    target_compile_definitions(tst_${PROJECT_NAME} INTERFACE -DLINUX)


    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message("Compiler is Clang")
        message(STATUS "CXX standard is: ${CMAKE_CXX_STANDARD}")

        # the following CLang options was tested on macOS
        set(clang_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
        set(clang_like_c "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU>")
        target_compile_options(${PROJECT_NAME} INTERFACE
                "$<${clang_like_cxx}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-attributes;-Wno-unused-private-field>>"
                "$<${clang_like_c}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-flexible-array-extensions;-Wno-zero-length-array;-Wno-c99-extensions>>"
        )

        # Unit test src files removes a few more..
        target_compile_options(tst_${PROJECT_NAME} INTERFACE
                "$<${clang_like_cxx}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-format;-Wno-sign-compare;-Wno-unused-function;-Wno-unused-variable;-Wno-attributes;-Wno-unused-variable;-Wno-unused-but-set-variable;-Wno-unused-lambda-capture>>"
                "$<${clang_like_c}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-flexible-array-extensions;-Wno-zero-length-array;-Wno-c99-extensions;-Wno-strict-flex-arrays;-Wno-format;-Wno-sign-compare;-Wno-unused-function;-Wno-unused-variable;-Wno-unused-but-set-variable;-Wno-unused-lambda-capture>>"
        )

    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Compiler is GCC")
        message(STATUS "CXX standard is: ${CMAKE_CXX_STANDARD}")
        #
        # See: https://discourse.cmake.org/t/how-to-specify-different-compile-options-for-c-and-c/583/5
        # and: https://cmake.org/cmake/help/v3.16/guide/tutorial/index.html#adding-generator-expressions-step-10
        #
        set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
        set(gcc_like_c "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU>")
        target_compile_options(${PROJECT_NAME} INTERFACE
                "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-attributes>>"
                "$<${gcc_like_c}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-flexible-array-extensions;-Wno-zero-length-array;-Wno-c99-extensions;-Wno-strict-flex-arrays>>"
        )

        # Unit test src files removes a few more..
        target_compile_options(tst_${PROJECT_NAME} INTERFACE
                "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-format;-Wno-sign-compare;-Wno-unused-function;-Wno-unused-variable;-Wno-attributes;-Wno-missing-field-initializers>>"
                "$<${gcc_like_c}:$<BUILD_INTERFACE:-Wno-unused-parameter;-Wno-unused-const-variable;-Wno-flexible-array-extensions;-Wno-zero-length-array;-Wno-c99-extensions;-Wno-strict-flex-arrays;-Wno-format;-Wno-sign-compare;-Wno-unused-function;-Wno-unused-variable>>"
        )
    endif()
endif()

target_include_directories(tst_${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

target_sources(${PROJECT_NAME} PUBLIC ${encdec_src})
target_sources(tst_${PROJECT_NAME} PUBLIC ${encdec_src} ${encdec_tst_src})

target_link_libraries(${PROJECT_NAME})
target_link_libraries(tst_${PROJECT_NAME})