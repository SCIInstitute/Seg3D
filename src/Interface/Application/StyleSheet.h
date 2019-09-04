/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

#ifndef INTERFACE_APPLICATION_STYLESHEET_H
#define INTERFACE_APPLICATION_STYLESHEET_H

#ifndef Q_MOC_RUN

//Qt Includes
#include <QtCore/QString>

#endif

namespace Seg3D
{
   
class StyleSheet
{
public:
  // == custom properties == //
    const static char* PALETTE_BACKGROUND_PROPERTY_C;
  
  // == Default Parent style == //
    const static QString MAIN_STYLE_C;

  // === GroupMenuBar === //
    const static QString GROUP_MENUBAR_C;
    
  // === StatusBar === //
    const static QString STATUSBAR_C;
    const static QString STATUSBAR_ERROR_C;
    const static QString STATUSBAR_WARNING_C;
    const static QString STATUSBAR_MESSAGE_C;
    const static QString STATUSBAR_DEBUG_C;
    
  // === ToolBoxWidget Styles === //
    const static QString TOOLBOXWIDGET_C;
    
  // === ToolBoxPageWidget Styles === //
    // - page_background_ - //
    const static QString TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_ACTIVE_C;
    const static QString TOOLBOXPAGEWIDGET_PAGE_BACKGROUND_INACTIVE_C;
    // - activate_button_ - //
    const static QString TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_ACTIVE_C;
    const static QString TOOLBOXPAGEWIDGET_ACTIVATE_BUTTON_INACTIVE_C;
  
  // === LayerManagerWidget Styles === //
    const static QString LAYERMANAGERWIDGET_C;
    const static QString BACKGROUND_COLOR_STR;
  
  // === LayerWidget Styles === //
    // - LayerWidget - //
    const static QString LAYERWIDGET_C;
    // - ACTIVE - //
  
//
    const static QString LAYER_WIDGET_BASE_ACTIVE_C;

    const static QString LAYER_WIDGET_BASE_ACTIVE_IN_USE_C;

    const static QString LAYER_WIDGET_BASE_IN_USE_C;

  // - INACTIVE - //
    const static QString LAYER_WIDGET_BASE_INACTIVE_C;
    const static QString LAYER_WIDGET_LABEL_INACTIVE_C;
    // - LOCKED - //
    const static QString LAYER_WIDGET_BASE_LOCKED_C;
    const static QString LAYER_WIDGET_LABEL_LOCKED_C;

    const static QString LAYER_WIDGET_LABEL_ACTIVE_C;   
    const static QString LAYER_WIDGET_LABEL_ACTIVE_IN_USE_C;    
    const static QString LAYER_WIDGET_LABEL_IN_USE_C; 
  
  // === LayerGroupWidget Styles === //
    // - ACTIVE - //
    const static QString GROUP_WIDGET_BACKGROUND_ACTIVE_C;
    const static QString GROUP_WIDGET_BACKGROUND_INACTIVE_C;

 // === LayerResamplerDialog Styles === //
    const static QString LAYER_RESAMPLER_C;

  // === PushDragButton's === //
    const static QString LAYER_PUSHDRAGBUTTON_C;
    const static QString GROUP_PUSHDRAGBUTTON_C;

  // === ViewerWidget === //
    const static QString VIEWERWIDGET_C;
    const static QString VIEWERSELECTED_C;
    const static QString VIEWERNOTSELECTED_C;
    const static QString VIEWERWIDGET_SINGLELINE_C;
    const static QString VIEWERWIDGET_DOUBLELINE_C;
    
  // == Rendering DockWidget == //
    const static QString RENDERINGWIDGET_C;
    const static QString RENDERING_CLIPPING_ACTIVE_C;
    const static QString RENDERING_CLIPPING_INACTIVE_C;   

    const static QString RENDERING_FOG_ACTIVE_C;
    const static QString RENDERING_FOG_INACTIVE_C;    

    const static QString RENDERING_VR_ACTIVE_C;
    const static QString RENDERING_VR_INACTIVE_C;   

  // == Project DockWidget == //
    const static QString PROJECTDOCKWIDGET_C;

  // == ProvenanceDockWidget == //
    const static QString PROVENANCE_LIST_HEADER_C;
    
    const static QString LAYERIO_C;

  // == SplashScreen == //
    const static QString SPLASHSCREEN_C;

  // == Preferences Interface == //
    const static QString PREFERENCES_C;

  // == Shortcuts Interface == //
    const static QString SHORTCUTS_C;

  // == Export Segmentations == //
    const static QString SEGMENTATION_EXPORT_C;
    const static QString QTLAYERLISTWIDGET_VALID_C;
    const static QString QTLAYERLISTWIDGET_INVALID_C;

  // == TriState Buttons == //
    const static QString TRISTATE_UNCHECKED_C;
    const static QString TRISTATE_INTERMEDIATE_C;
    const static QString TRISTATE_CHECKED_C;

  // == Speedline Tool == //
    const static QString SPEEDLINE_TOOL_C;

  // == Colors == //
    const static QString ACCENT_COLOR_C;
    const static QString CRITICAL_ERROR_COLOR_C;
    const static QString ERROR_COLOR_C;
    const static QString WARNING_COLOR_C;
    const static QString MESSAGE_COLOR_C;
    const static QString SUCCESS_COLOR_C;
    const static QString DEBUG_COLOR_C;
};    

}  // end namespace Seg3D

#endif
