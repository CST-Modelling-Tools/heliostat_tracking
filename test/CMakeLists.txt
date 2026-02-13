set(This heliostat_tracking_tests)

set(Sources
    ArmatureJointTests.cpp
    ElevationAngleKMTests.cpp
    gcfTests.cpp
    HourAngleKMTests.cpp
    IntervalTests.cpp
    IntervalPeriodicTests.cpp
    Matrix4x4Tests.cpp
    TrackerArmature2ATests.cpp
    TrackerSolver2ATests.cpp
    TrackerTargetTests.cpp
    TransformTests.cpp
    vec2dTests.cpp
    vec3dTests.cpp
)

# Only add Python-related tests if Python wrappers are enabled
if(PYTHON_WRAPPER_ENABLED)
    list(APPEND Sources PythonWrapperTests.cpp)
    find_package(pybind11 CONFIG REQUIRED)
endif()

add_executable(${This} ${Sources})

target_link_libraries(${This} PRIVATE
    gtest_main
    gmock_main
    heliostat_tracking
)

# -------------------------------------------------------------------
# Python embed support for tests
# -------------------------------------------------------------------
if(PYTHON_WRAPPER_ENABLED)
    target_link_libraries(${This} PRIVATE pybind11::embed)

    # On Windows, ensure pythonXY.dll is next to the test executable.
    # PYTHON_RUNTIME_DLL_PATH is computed in the top-level CMakeLists.txt.
    if (WIN32 AND DEFINED PYTHON_RUNTIME_DLL_PATH AND PYTHON_RUNTIME_DLL_PATH)
        add_custom_command(TARGET ${This} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${PYTHON_RUNTIME_DLL_PATH}"
                    "$<TARGET_FILE_DIR:${This}>"
            VERBATIM
        )
    endif()

    # -------------------------------------------------------------------
    # CRITICAL WINDOWS FIX:
    # Generate pythonXY._pth next to the test executable so embedded Python
    # can find the standard library (encodings, etc.) reliably.
    # This avoids "Failed to import encodings module".
    # -------------------------------------------------------------------
    if (WIN32)
        # Example: Python 3.14 -> python314._pth
        set(_PYXY "${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
        set(_PTH_NAME "python${_PYXY}._pth")

        # One entry per line. Keep test dir first so the .pyd is importable.
        # 'import site' enables site-packages processing.
        set(_PTH_CONTENT
"$<TARGET_FILE_DIR:${This}>
${PYTHON_STDLIB_DIR}
${PYTHON_VENV_SITEPACKAGES}
import site
")

        file(GENERATE
            OUTPUT "$<TARGET_FILE_DIR:${This}>/${_PTH_NAME}"
            CONTENT "${_PTH_CONTENT}"
        )

        unset(_PYXY)
        unset(_PTH_NAME)
        unset(_PTH_CONTENT)
    endif()
endif()

# -------------------------------------------------------------------
# Register tests with CTest
# -------------------------------------------------------------------
include(GoogleTest)

# Attach environment vars to discovered tests
set(_GTEST_ENV_VARS "")

if (PYTHON_WRAPPER_ENABLED)
    set(_TEST_DIR "$<TARGET_FILE_DIR:${This}>")

    if (WIN32)
        # Build PYTHONPATH for non-_pth cases and general robustness.
        set(_PYTHONPATH_WIN "${_TEST_DIR}")
        if (DEFINED PYTHON_STDLIB_DIR AND PYTHON_STDLIB_DIR)
            set(_PYTHONPATH_WIN "${_PYTHONPATH_WIN};${PYTHON_STDLIB_DIR}")
        endif()
        if (DEFINED PYTHON_VENV_SITEPACKAGES AND PYTHON_VENV_SITEPACKAGES)
            set(_PYTHONPATH_WIN "${_PYTHONPATH_WIN};${PYTHON_VENV_SITEPACKAGES}")
        endif()

        # Escape ';' because ENVIRONMENT is a CMake list.
        string(REPLACE ";" "\\;" _PYTHONPATH_WIN_ESCAPED "${_PYTHONPATH_WIN}")
        list(APPEND _GTEST_ENV_VARS "PYTHONPATH=${_PYTHONPATH_WIN_ESCAPED}")

        # Setting PYTHONHOME helps some layouts; harmless otherwise.
        if (DEFINED PYTHON_HOME_DIR AND PYTHON_HOME_DIR)
            list(APPEND _GTEST_ENV_VARS "PYTHONHOME=${PYTHON_HOME_DIR}")
        endif()

        unset(_PYTHONPATH_WIN)
        unset(_PYTHONPATH_WIN_ESCAPED)
    else()
        set(_PYTHONPATH_UNIX "${_TEST_DIR}")
        if (DEFINED PYTHON_VENV_SITEPACKAGES AND PYTHON_VENV_SITEPACKAGES)
            set(_PYTHONPATH_UNIX "${_PYTHONPATH_UNIX}:${PYTHON_VENV_SITEPACKAGES}")
        endif()
        list(APPEND _GTEST_ENV_VARS "PYTHONPATH=${_PYTHONPATH_UNIX}:$ENV{PYTHONPATH}")
        unset(_PYTHONPATH_UNIX)
    endif()

    unset(_TEST_DIR)
endif()

# Discovery during ctest run, not during build (prevents "no such file" at link time)
gtest_discover_tests(${This}
    DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${This}>"
    PROPERTIES
        ENVIRONMENT "${_GTEST_ENV_VARS}"
)

unset(_GTEST_ENV_VARS)