#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2011 Scientific Computing and Imaging Institute,
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

# Define macros for adding documentation
# Migrated from the SCIRun documentation build

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/seg3d.sty.in  ${CMAKE_CURRENT_BINARY_DIR}/seg3d.sty)

MESSAGE(STATUS "Setting up documentation")

MACRO(ADD_PDF_DOCUMENT document)

  MESSAGE(STATUS "Create ${document} Document")

  ADD_CUSTOM_COMMAND(
      OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${document}.aux
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
      COMMAND ${PDFLATEX_COMPILER}
      ARGS    -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
      COMMENT "PDFLatex ${document} (first pass)"
    )

  FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/${document}.tex tex_file)
  STRING(REGEX MATCH "bibliography" NEED_BIBTEX ${tex_file})


  IF (NEED_BIBTEX)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${document}.bbl
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.aux
        COMMAND ${BIBTEX_COMPILER}
        ARGS    -terse ${CMAKE_CURRENT_BINARY_DIR}/${document}
        COMMENT "Bibtex ${document}"
      )
      
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${document}.pdf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.bbl
        COMMAND ${PDFLATEX_COMPILER}
        ARGS    -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
        COMMENT "PDFLatex ${document} (second pass)"  
      )
      
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${document}.log
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.bbl
                ${CMAKE_CURRENT_BINARY_DIR}/${document}.pdf
        COMMAND ${PDFLATEX_COMPILER}
        ARGS    -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
        COMMENT "PDFLatex ${document} (third pass)"  
      )
  ELSE (NEED_BIBTEX)
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${document}.pdf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.aux
        COMMAND ${PDFLATEX_COMPILER}
        ARGS    -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
        COMMENT "PDFLatex ${document} (second pass)"  
      )
      
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${document}.log
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.pdf
        COMMAND ${PDFLATEX_COMPILER}
        ARGS    -interaction=batchmode ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex
        COMMENT "PDFLatex ${document} (third pass)"  
      )

  ENDIF (NEED_BIBTEX)

  ADD_CUSTOM_TARGET(${document} ALL 
                      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${document}.log)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${document}.tex  ${CMAKE_CURRENT_BINARY_DIR}/${document}.tex COPYONLY)

  FILE(GLOB_RECURSE png_figures "${CMAKE_CURRENT_SOURCE_DIR}/${document}_figures/*.png")

  FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${document}_figures)
  FOREACH(png_figure ${png_figures})
    MESSAGE(STATUS "copying ${png_figure}")
    GET_FILENAME_COMPONENT(png_name ${png_figure} NAME)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${document}_figures/${png_name}
      ${CMAKE_CURRENT_BINARY_DIR}/${document}_figures/${png_name} COPYONLY)
  ENDFOREACH(png_figure ${png_figures})

  FILE(GLOB_RECURSE pdf_figures "${CMAKE_CURRENT_SOURCE_DIR}/${document}_figures/*.pdf")
  FOREACH(pdf_figure ${pdf_figures})
    GET_FILENAME_COMPONENT(pdf_name ${pdf_figure} NAME)
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${document}_figures/${pdf_name}
      ${CMAKE_CURRENT_BINARY_DIR}/${document}_figures/${pdf_name} COPYONLY)
  ENDFOREACH(pdf_figure ${pdf_figures})

  IF(APPLE)
    INSTALL(FILES ${SEG3D_BINARY_DIR}/Documentation/${document}.pdf DESTINATION ${OSX_UTILS_DIR})
  ELSE()
    INSTALL(FILES ${SEG3D_BINARY_DIR}/Documentation/${document}.pdf DESTINATION Documentation)
  ENDIF()
ENDMACRO(ADD_PDF_DOCUMENT)


# ADD DOCUMENTS

ADD_PDF_DOCUMENT(Seg3DTutorial)
ADD_PDF_DOCUMENT(Seg3DBasicFunctionality)
#ADD_PDF_DOCUMENT(Seg3DPythonCookbook)

# Forcing documentation build
