#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2016 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software. 
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

function( GENERATE_REVISION_INFO )
  string(TOUPPER "${PROJECT_NAME}" PROJECT_UPPERCASE_NAME)

  include(${CMAKE_SOURCE_DIR}/CMake/GetGitRevisionDescription.cmake)

  git_describe(GIT_TAG --tag)
  git_describe(REVISION_BRANCH --all)
  get_git_head_revision(REVISION_REFSPEC REVISION_HASHVAR)
  string(SUBSTRING ${REVISION_HASHVAR} 0 10 TRUNC_HASHVAR)
  git_show_last_commit_date(GIT_LAST_COMMIT)

  message(STATUS "Git version branch: ${REVISION_BRANCH}")
  message(STATUS "Git version tag: ${GIT_TAG}")
  message(STATUS "Git last commit refspec and hash: ${REVISION_REFSPEC} ${REVISION_HASHVAR}")
  message(STATUS "Git truncated hash: ${TRUNC_HASHVAR}")
  message(STATUS "Git last commit date: ${GIT_LAST_COMMIT}")

  set( GIT_REVISION_INFO ${TRUNC_HASHVAR} )

  if(REVISION_BRANCH MATCHES "-NOTFOUND")
    if(NOT GIT_TAG MATCHES "-NOTFOUND")
      set( GIT_REVISION_INFO ${GIT_TAG })
    endif()
  endif()

  set( GIT_BUILD_INFO "${REVISION_BRANCH} ${TRUNC_HASHVAR}" CACHE INTERNAL "Git build information string." FORCE )

  set( INFO_STRING "#define GIT_${PROJECT_UPPERCASE_NAME}_REVISION \"${GIT_REVISION_INFO}\"\n" )
  set( INFO_STRING "${INFO_STRING}#define GIT_${PROJECT_UPPERCASE_NAME}_DATE \"${GIT_LAST_COMMIT}\"\n" )
  set( INFO_STRING "${INFO_STRING}#define GIT_${PROJECT_UPPERCASE_NAME}_REVISIONINFO \"${PROJECT_NAME} ${GIT_BUILD_INFO}\"\n\n" )

  #if( Subversion_FOUND AND EXISTS ${PROJECT_BINARY_DIR}/.svn )
  #else( Subversion_FOUND AND EXISTS ${PROJECT_BINARY_DIR}/.svn )
  #  set( INFO_STRING "#define ${PROJECT_UPPERCASE_NAME}_REVISION \"unknown\"\n" )
  #  set( INFO_STRING "${INFO_STRING}#define ${PROJECT_UPPERCASE_NAME}_DATE \"unknown\"\n" )
  #  set( INFO_STRING "${INFO_STRING}#define ${PROJECT_UPPERCASE_NAME}_REVISIONINFO \"${PROJECT_NAME} Revision:unknown\"\n\n" )
  #endif()

  set( fileNAME "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_RevisionInfo.h" )
  file( WRITE ${fileNAME} ${INFO_STRING})
endfunction()
