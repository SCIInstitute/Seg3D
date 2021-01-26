# - Use Module for QT4
# Sets up C and C++ to use Qt 4.  It is assumed that FindQt.cmake
# has already been loaded.  See FindQt.cmake for information on
# how to load Qt 4 into your CMake project.


add_definitions(${QT_DEFINITIONS})

include_directories(${QT_INCLUDE_DIR})

set(QT_LIBRARIES "")

if (QT_USE_QTMAIN)
  if (WIN32)
    set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})
  endif ()
endif ()

# Macro for setting up compile flags for Qt modules
macro(QT_MODULE_SETUP module)
  if (QT_QT${module}_FOUND)
    add_definitions(-DQT_${module}_LIB)
    include_directories(${QT_QT${module}_INCLUDE_DIR})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_QT${module}_LIBRARY})
    set(QT_LIBRARIES ${QT_LIBRARIES} ${QT_${module}_LIB_DEPENDENCIES})
  else ()
    message("Qt ${module} library not found.")
  endif ()
endmacro()


# Qt modules  (in order of dependence)

if (QT_USE_QT3SUPPORT)
  QT_MODULE_SETUP(3SUPPORT)
    add_definitions(-DQT3_SUPPORT)
endif ()

if (QT_USE_QTOPENGL)
  QT_MODULE_SETUP(OPENGL)
endif ()

if (QT_USE_QTASSISTANT)
  QT_MODULE_SETUP(ASSISTANT)
endif ()

if (QT_USE_QTDESIGNER)
  QT_MODULE_SETUP(DESIGNER)
endif ()

if (QT_USE_QTMOTIF)
  QT_MODULE_SETUP(MOTIF)
endif ()

if (QT_USE_QTNSPLUGIN)
  QT_MODULE_SETUP(NSPLUGIN)
endif ()

if (QT_USE_QTSVG)
  QT_MODULE_SETUP(SVG)
endif ()

if (QT_USE_QTUITOOLS)
  QT_MODULE_SETUP(UITOOLS)
endif ()

if (NOT QT_DONT_USE_QTGUI)
  QT_MODULE_SETUP(GUI)
endif ()

if (QT_USE_QTTEST)
  QT_MODULE_SETUP(TEST)
endif ()

if (QT_USE_QTXML)
  QT_MODULE_SETUP(XML)
endif ()

if (QT_USE_QTSQL)
  QT_MODULE_SETUP(SQL)
endif ()

if (QT_USE_QTNETWORK)
  QT_MODULE_SETUP(NETWORK)
endif ()

if (NOT QT_DONT_USE_QTCORE)
  QT_MODULE_SETUP(CORE)
endif ()


