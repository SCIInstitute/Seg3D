/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYERIO_MRCLAYEREXPORTER_H
#define APPLICATION_LAYERIO_MRCLAYEREXPORTER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <boost/filesystem.hpp>


// Application includes
#include <Application/LayerIO/LayerExporter.h>
#include <Application/LayerIO/LayerIO.h>

namespace Seg3D
{
  
  class MRCLayerExporter : public LayerExporter
  {
    SEG3D_EXPORTER_TYPE( "MRC Exporter", ".mrc" )
    
    // -- Constructor/Destructor --
  public:
    /// Construct a new layer file importer
    MRCLayerExporter( std::vector< LayerHandle >& layers );
    
    /// Virtual destructor for memory management of derived classes
    virtual ~MRCLayerExporter() {}
    
    // --Import the data as a specific type --  
  public: 
    
    /// EXPORT_LAYER
    /// Export the layer to file
    virtual bool export_layer( const std::string& mode, const std::string& file_path, 
                               const std::string& name ) override;
    
    virtual void set_label_layer_values( std::vector< double > values ) override
    { 
      this->label_values_ = values;
    }

    virtual bool label_layer_values_set() override { return (! this->label_values_.empty() ); }

  private:
    bool export_mrc(  const std::string& file_path );
    bool export_single_masks( const std::string& file_path );
    bool export_mask_label( const std::string& file_path );

  private:
    std::vector< double > label_values_;
  };
  
} // end namespace seg3D

#endif
