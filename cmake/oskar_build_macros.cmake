#
# cmake/oskar_build_macros.cmake
#
#===============================================================================
# A collection of cmake macros used by the oskar build system.
#


# Macro to build and install oskar mex functions.
macro(oskar_mex name source)

    #message("= INFO: Building mex function: ${name} (${source})")

    # TODO: Add script to find MATLAB and MATLAB_FOUND guard?
    # TODO: Add critical fail on matlab not found.
    # TODO: Add critical fail on not having set other required variables.

    # Find out if this is a CUDA or C/C++ source file based on the extension
    # and use the appropriate add_library target.
    get_filename_component(ext ${source} EXT)
    if (${ext} STREQUAL ".cpp" OR ${ext} STREQUAL "*.c")
        add_library(${name} SHARED ${source} mex_function.def)
    elseif (${ext} STREQUAL ".cu")
        cuda_add_library(${name} SHARED ${source} mex_function.def)
    else ()
        message(CRITICAL "OSKAR_MEX: UNRECOGNISED SOURCE FILE EXTENSION!")
    endif ()

    target_link_libraries(${name} oskar ${MATLAB_LIBRARIES})
    set_target_properties(${name} PROPERTIES
        PREFIX "" SUFFIX ".${MATLAB_MEXFILE_EXT}"
        INSTALL_RPATH ${OSKAR_LIB_INSTALL_DIR}
        INSTALL_RPATH_USE_LINK_PATH TRUE
        COMPILE_FLAGS ${MATLAB_CXX_FLAGS}
        LINK_FLAGS ${MATLAB_CXX_FLAGS})
    install(TARGETS ${name} DESTINATION ${OSKAR_MATLAB_INSTALL_DIR})
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION "${OSKAR_MATLAB_INSTALL_DIR}/.."
        FILES_MATCHING PATTERN "*.m"
        PATTERN ".svn" EXCLUDE)

endmacro(oskar_mex)

