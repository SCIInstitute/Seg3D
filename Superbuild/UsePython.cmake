
include_directories(${SCI_PYTHON_INCLUDE})
link_directories(${SCI_PYTHON_LINK_LIBRARY_DIRS})
add_definitions(-DBUILD_WITH_PYTHON)
if(WIN32)
  add_definitions(-DPy_ENABLE_SHARED=1)
endif()
