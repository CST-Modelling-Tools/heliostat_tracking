cmake_minimum_required(VERSION 3.25.1)

set(This heliostat_tracking)
project(${This} VERSION 0.1.0 LANGUAGES CXX)

# Enable CTest / dashboard support (generates DartConfiguration.tcl)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

# Make default visibility hidden for all targets
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

# ----------------------------
# Build options (portable)
# ----------------------------
option(HELIOS_BUILD_TESTS "Build unit tests" ON)
option(HELIOS_BUILD_EXAMPLES "Build example executables" ON)
option(HELIOS_BUILD_PYTHON "Build Python pybind11 wrapper" ON)
option(HELIOS_BUILD_MATHEMATICA "Build Mathematica wrapper if headers are available" ON)

# Add subdirectory for Google Test (only if we build tests)
if (HELIOS_BUILD_TESTS)
    add_subdirectory(googletest)
endif()

set(Headers
    ./include/ArmatureJoint.h
    ./include/ElevationAngleKM.h
    ./include/gcf.h
    ./include/HourAngleKM.h
    ./include/Interval.h
    ./include/IntervalPeriodic.h
    ./include/Matrix4x4.h
    ./include/Ray.h
    ./include/TrackerArmature2A.h
    ./include/TrackerSolver2A.h
    ./include/TrackerTarget.h
    ./include/Transform.h
    ./include/vec2d.h
    ./include/vec3d.h
)

set(Sources
    ArmatureJoint.cpp
    ElevationAngleKM.cpp
    gfc.cpp
    HourAngleKM.cpp
    Interval.cpp
    IntervalPeriodic.cpp
    Matrix4x4.cpp
    TrackerTarget.cpp
    TrackerArmature2A.cpp
    TrackerSolver2A.cpp
    Transform.cpp
    vec2d.cpp
    vec3d.cpp
)

add_library(${This} SHARED ${Sources} ${Headers})

# Prefer target-scoped includes (more maintainable across platforms/targets)
target_include_directories(${This}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
)

# ----------------------------
# Mathematica (optional, portable)
# ----------------------------
set(Mathematica_INCLUDE_DIR "" CACHE PATH "Path to Mathematica/Wolfram include directory containing WolframLibrary.h")

if (HELIOS_BUILD_MATHEMATICA)
    if (NOT Mathematica_INCLUDE_DIR)
        if (WIN32)
            # Deterministic fallback for common Windows installs (avoids find_path quirks)
            set(_MATH_CANDIDATE "C:/Program Files/Wolfram Research/Mathematica/13.3/SystemFiles/IncludeFiles/C")
            if (EXISTS "${_MATH_CANDIDATE}/WolframLibrary.h")
                set(Mathematica_INCLUDE_DIR "${_MATH_CANDIDATE}" CACHE PATH "Path to Mathematica/Wolfram include directory containing WolframLibrary.h" FORCE)
            endif()
            unset(_MATH_CANDIDATE)

            # If still not set, try additional known locations
            if (NOT Mathematica_INCLUDE_DIR)
                find_path(Mathematica_INCLUDE_DIR
                    NAMES WolframLibrary.h
                    PATHS
                        "C:/Program Files/Wolfram Research/Mathematica/13.3/SystemFiles/IncludeFiles/C"
                        "C:/Program Files/Wolfram Research/Mathematica/14.0/SystemFiles/IncludeFiles/C"
                        "C:/Program Files/Wolfram Research/Wolfram Engine/13.3/SystemFiles/IncludeFiles/C"
                        "C:/Program Files/Wolfram Research/Wolfram Engine/14.0/SystemFiles/IncludeFiles/C"
                        "C:/Program Files/Wolfram Research/Mathematica/SystemFiles/IncludeFiles/C"
                        "C:/Program Files/Wolfram Research/Wolfram Engine/SystemFiles/IncludeFiles/C"
                )
            endif()
        else()
            # macOS/Linux: search common install roots using suffixes (no globs)
            find_path(Mathematica_INCLUDE_DIR
                NAMES WolframLibrary.h
                PATHS
                    "/Applications/Mathematica.app/Contents"
                    "/usr/local/Wolfram/Mathematica"
                    "/usr/local/Wolfram/WolframEngine"
                    "/opt/Wolfram/Mathematica"
                    "/opt/Wolfram/WolframEngine"
                PATH_SUFFIXES
                    "SystemFiles/IncludeFiles/C"
            )
        endif()
    endif()

    if (Mathematica_INCLUDE_DIR AND EXISTS "${Mathematica_INCLUDE_DIR}/WolframLibrary.h")
        message(STATUS "Found Mathematica include directory: ${Mathematica_INCLUDE_DIR}")
        message(STATUS "Generating Mathematica wrappers")
        target_include_directories(${This} PRIVATE "${Mathematica_INCLUDE_DIR}")
        target_sources(${This} PRIVATE MathematicaWrapper.cpp)
    else()
        message(STATUS "Mathematica include directory not found; Mathematica wrapper will not be built.")
    endif()
else()
    message(STATUS "HELIOS_BUILD_MATHEMATICA=OFF; Mathematica wrapper disabled.")
endif()

# ----------------------------
# Python + pybind11 (optional, portable)
# ----------------------------
set(PYTHON_WRAPPER_ENABLED FALSE)

# Used by Windows to make embedded-Python tests and .pyd imports work reliably
set(PYTHON_RUNTIME_DLL_PATH "")
set(PYTHON_HOME_DIR "")
set(PYTHON_STDLIB_DIR "")
set(PYTHON_VENV_SITEPACKAGES "")

if (HELIOS_BUILD_PYTHON)
    find_package(Python COMPONENTS Interpreter Development)
    if (Python_FOUND)
        message(STATUS "Found Python interpreter at: ${Python_EXECUTABLE}")

        # On Windows, embedded Python may require pythonXY.dll at runtime.
        # Also compute Python "home"/stdlib paths so tests can set PYTHONHOME/PYTHONPATH.
        if (WIN32)
            execute_process(
                COMMAND ${Python_EXECUTABLE} -c
                    "import os, sysconfig; print(os.path.join(sysconfig.get_config_var('installed_base'), sysconfig.get_config_var('LDLIBRARY')))"
                OUTPUT_VARIABLE PYTHON_RUNTIME_DLL_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if (EXISTS "${PYTHON_RUNTIME_DLL_PATH}")
                message(STATUS "Python runtime DLL: ${PYTHON_RUNTIME_DLL_PATH}")
            else()
                message(WARNING "Python runtime DLL path computed but file not found: ${PYTHON_RUNTIME_DLL_PATH}")
                set(PYTHON_RUNTIME_DLL_PATH "")
            endif()

            # Base install (where stdlib lives)
            execute_process(
                COMMAND ${Python_EXECUTABLE} -c
                    "import sysconfig; print(sysconfig.get_config_var('installed_base') or '')"
                OUTPUT_VARIABLE PYTHON_HOME_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            # Stdlib directory (encodings, etc.)
            execute_process(
                COMMAND ${Python_EXECUTABLE} -c
                    "import os, sysconfig; b=(sysconfig.get_config_var('installed_base') or ''); print(os.path.join(b,'Lib') if b else '')"
                OUTPUT_VARIABLE PYTHON_STDLIB_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            # Venv site-packages directory (where pybind11 and your deps live)
            execute_process(
                COMMAND ${Python_EXECUTABLE} -c
                    "import os, sys; print(os.path.join(sys.prefix,'Lib','site-packages'))"
                OUTPUT_VARIABLE PYTHON_VENV_SITEPACKAGES
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            if (PYTHON_HOME_DIR)
                message(STATUS "Python home dir: ${PYTHON_HOME_DIR}")
            endif()
            if (PYTHON_STDLIB_DIR)
                message(STATUS "Python stdlib dir: ${PYTHON_STDLIB_DIR}")
            endif()
            if (PYTHON_VENV_SITEPACKAGES)
                message(STATUS "Python venv site-packages: ${PYTHON_VENV_SITEPACKAGES}")
            endif()
        endif()

        # Discover pybind11 include and cmake dir via python module
        execute_process(
            COMMAND ${Python_EXECUTABLE} -c "import pybind11; print(pybind11.get_include())"
            OUTPUT_VARIABLE PYBIND11_INCLUDE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if (PYBIND11_INCLUDE_DIR AND EXISTS "${PYBIND11_INCLUDE_DIR}/pybind11/embed.h")
            message(STATUS "Found pybind11 include directory: ${PYBIND11_INCLUDE_DIR}")

            execute_process(
                COMMAND ${Python_EXECUTABLE} -m pybind11 --cmakedir
                OUTPUT_VARIABLE pybind11_DIR
                ERROR_VARIABLE error
                RESULT_VARIABLE result
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
            )

            if (result EQUAL 0 AND EXISTS "${pybind11_DIR}/pybind11Config.cmake")
                message(STATUS "Found pybind11 at: ${pybind11_DIR}")
                list(APPEND CMAKE_PREFIX_PATH ${pybind11_DIR})

                find_package(pybind11 CONFIG REQUIRED)

                pybind11_add_module(heliostat_tracking_module PythonWrapper.cpp)
                target_link_libraries(heliostat_tracking_module PRIVATE ${This})

                # Ensure module can be imported on Windows even if python dll dir isn't on PATH
                if (WIN32 AND PYTHON_RUNTIME_DLL_PATH)
                    add_custom_command(TARGET heliostat_tracking_module POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                "${PYTHON_RUNTIME_DLL_PATH}"
                                "$<TARGET_FILE_DIR:heliostat_tracking_module>"
                        VERBATIM
                    )
                endif()

                set(PYTHON_WRAPPER_ENABLED TRUE)
            else()
                message(WARNING "pybind11 cmake dir not found via Python. Error: ${error}")
            endif()
        else()
            message(WARNING "pybind11 not importable in the selected Python environment; Python wrapper will not be built.")
        endif()
    else()
        message(STATUS "Python (Interpreter+Development) not found; Python wrapper will not be built.")
    endif()
else()
    message(STATUS "HELIOS_BUILD_PYTHON=OFF; Python wrapper disabled.")
endif()

include(GenerateExportHeader)
generate_export_header(${This})

# Tests/examples can use PYTHON_WRAPPER_ENABLED and the PYTHON_* variables above
if (HELIOS_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (HELIOS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()