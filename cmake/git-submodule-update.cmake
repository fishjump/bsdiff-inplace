function(GitSubmoduleUpdate)
    find_package(Git QUIET)

    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git")
        message("Cannot find git repository, skipped")
        return()
    endif()

    if(NOT GIT_FOUND)
        message(FATAL_ERROR "Cannot find git executable")
        return()
    endif()

    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endfunction(GitSubmoduleUpdate)
