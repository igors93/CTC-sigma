cmake_minimum_required(VERSION 3.20)
project(ctc_sigma VERSION 0.1.0 LANGUAGES C)

option(CTC_SIGMA_BUILD_SHARED "Build the shared library used by Python tests" ON)
option(CTC_SIGMA_BUILD_TOOLS "Build command-line development tools" ON)
option(CTC_SIGMA_ENABLE_STRICT_WARNINGS "Enable strict compiler warnings" ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(cmake/CompilerWarnings.cmake)

set(CTC_SIGMA_SOURCES
    src/arith.c
    src/braid.c
    src/branch.c
    src/constants.c
    src/ctc_sigma.c
    src/encoder.c
    src/field.c
    src/fold.c
    src/keccak.c
    src/lehmer.c
    src/permutation.c
    src/sponge.c
)

add_library(ctc_sigma_static STATIC ${CTC_SIGMA_SOURCES})
add_library(ctc_sigma::static ALIAS ctc_sigma_static)
set_target_properties(ctc_sigma_static PROPERTIES OUTPUT_NAME ctc_sigma)
target_include_directories(ctc_sigma_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
if(CTC_SIGMA_ENABLE_STRICT_WARNINGS)
    ctc_sigma_set_warnings(ctc_sigma_static)
endif()

if(CTC_SIGMA_BUILD_SHARED)
    add_library(ctc_sigma_shared SHARED ${CTC_SIGMA_SOURCES})
    add_library(ctc_sigma::shared ALIAS ctc_sigma_shared)
    set_target_properties(ctc_sigma_shared PROPERTIES
        OUTPUT_NAME ctc_sigma
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
    )
    target_include_directories(ctc_sigma_shared
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    if(CTC_SIGMA_ENABLE_STRICT_WARNINGS)
        ctc_sigma_set_warnings(ctc_sigma_shared)
    endif()
endif()

if(CTC_SIGMA_BUILD_TOOLS)
    add_executable(ctc_dump_constants tools/dump_constants.c)
    target_link_libraries(ctc_dump_constants PRIVATE ctc_sigma::static)
    if(CTC_SIGMA_ENABLE_STRICT_WARNINGS)
        ctc_sigma_set_warnings(ctc_dump_constants)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS ctc_sigma_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
if(CTC_SIGMA_BUILD_SHARED)
    install(TARGETS ctc_sigma_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
