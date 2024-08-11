set(FOLDER_NAME "matchwatcher")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

COLMAP_ADD_EXECUTABLE(
    NAME matchwatcher
    SRCS
        imageholder.cc
        imageholder.h
        imagescene.cc
        imagescene.h
        imageview.cc
        imageview.h
        mainwindow.ui
        mainwindow.cc
        mainwindow.h
        marker.cc
        marker.h
        points.h
        main.cc
    LINK_LIBS
        colmap_controllers
        colmap_retrieval
        colmap_scene
        colmap_sfm
        colmap_util
        ${OPTIONAL_LIBS}
        Boost::boost
        Qt5::Widgets
        Eigen3::Eigen
)
