if(POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
endif()

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

# Determine project compiler.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(IS_MSVC TRUE)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(IS_GNU TRUE)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(IS_CLANG TRUE)
endif()

# Determine project architecture.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "[ix].?86|amd64|AMD64")
    set(IS_X86 TRUE)
endif()

# Determine project operating system.
string(REGEX MATCH "Linux" IS_LINUX ${CMAKE_SYSTEM_NAME})
string(REGEX MATCH "DragonFly|BSD" IS_BSD ${CMAKE_SYSTEM_NAME})
string(REGEX MATCH "SunOS" IS_SOLARIS ${CMAKE_SYSTEM_NAME})
if(WIN32)
    set(IS_WINDOWS TRUE BOOL INTERNAL)
endif()
if(APPLE)
    set(IS_MACOS TRUE BOOL INTERNAL)
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug"
   OR CMAKE_BUILD_TYPE_LOWER STREQUAL "relwithdebinfo")
    set(IS_DEBUG TRUE)
endif()

# Enable solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_TARGETS_ROOT_FOLDER "cmake")
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER
             ${CMAKE_TARGETS_ROOT_FOLDER})
set(COLMAP_TARGETS_ROOT_FOLDER "colmap_targets")

# Replacement for the normal add_library() command. The syntax remains the same
# in that the first argument is the target name, and the following arguments
# are the source files to use when building the target.
macro(COLMAP_ADD_LIBRARY)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs NAME SRCS PRIVATE_LINK_LIBS PUBLIC_LINK_LIBS)
    cmake_parse_arguments(COLMAP_ADD_LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_library(${COLMAP_ADD_LIBRARY_NAME} STATIC ${COLMAP_ADD_LIBRARY_SRCS})
    set_target_properties(${COLMAP_ADD_LIBRARY_NAME} PROPERTIES FOLDER
        ${COLMAP_TARGETS_ROOT_FOLDER}/${FOLDER_NAME})
    if(CLANG_TIDY_EXE)
        set_target_properties(${COLMAP_ADD_LIBRARY_NAME}
            PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*")
    endif()
    target_link_libraries(${COLMAP_ADD_LIBRARY_NAME}
        PRIVATE ${COLMAP_ADD_LIBRARY_PRIVATE_LINK_LIBS}
        PUBLIC ${COLMAP_ADD_LIBRARY_PUBLIC_LINK_LIBS})
endmacro(COLMAP_ADD_LIBRARY)

# Replacement for the normal add_executable() command. The syntax remains the
# same in that the first argument is the target name, and the following
# arguments are the source files to use when building the target.
macro(COLMAP_ADD_EXECUTABLE)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs NAME SRCS LINK_LIBS)
    cmake_parse_arguments(COLMAP_ADD_EXECUTABLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    add_executable(${COLMAP_ADD_EXECUTABLE_NAME} ${COLMAP_ADD_EXECUTABLE_SRCS})
    set_target_properties(${COLMAP_ADD_EXECUTABLE_NAME} PROPERTIES FOLDER
        ${COLMAP_TARGETS_ROOT_FOLDER}/${FOLDER_NAME})
    target_link_libraries(${COLMAP_ADD_EXECUTABLE_NAME} ${COLMAP_ADD_EXECUTABLE_LINK_LIBS})
    if(VCPKG_BUILD)
        install(TARGETS ${COLMAP_ADD_EXECUTABLE_NAME} DESTINATION tools/)
    else()
        install(TARGETS ${COLMAP_ADD_EXECUTABLE_NAME} DESTINATION bin/)
    endif()
    if(CLANG_TIDY_EXE)
        set_target_properties(${COLMAP_ADD_EXECUTABLE_NAME}
            PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*")
    endif()
endmacro(COLMAP_ADD_EXECUTABLE)

# Wrapper for test executables.
macro(COLMAP_ADD_TEST)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs NAME SRCS LINK_LIBS)
    cmake_parse_arguments(COLMAP_ADD_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(TESTS_ENABLED)
        # ${ARGN} will store the list of link libraries.
        set(COLMAP_ADD_TEST_NAME "colmap_${FOLDER_NAME}_${COLMAP_ADD_TEST_NAME}")
        add_executable(${COLMAP_ADD_TEST_NAME} ${COLMAP_ADD_TEST_SRCS})
        set_target_properties(${COLMAP_ADD_TEST_NAME} PROPERTIES FOLDER
            ${COLMAP_TARGETS_ROOT_FOLDER}/${FOLDER_NAME})
        if(CLANG_TIDY_EXE)
            set_target_properties(${COLMAP_ADD_TEST_NAME}
                PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=.*")
        endif()
        target_link_libraries(${COLMAP_ADD_TEST_NAME}
            ${COLMAP_ADD_TEST_LINK_LIBS}
            GTest::gtest
            GTest::gtest_main)
        add_test("${FOLDER_NAME}/${COLMAP_ADD_TEST_NAME}" ${COLMAP_ADD_TEST_NAME})
        if(IS_MSVC)
            install(TARGETS ${COLMAP_ADD_TEST_NAME} DESTINATION bin/)
        endif()
    endif()
endmacro(COLMAP_ADD_TEST)
