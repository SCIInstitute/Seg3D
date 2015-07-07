/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYERIO_LAYERIO_H
#define APPLICATION_LAYERIO_LAYERIO_H

// Core includes
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerIO/LayerSingleFileImporter.h>
#include <Application/LayerIO/LayerFileSeriesImporter.h>
#include <Application/LayerIO/LayerExporter.h>
#include <Application/LayerIO/LayerImporterInfo.h>
#include <Application/LayerIO/LayerExporterInfo.h>

namespace Seg3D
{
/// LAYERIO:
/// This class is the factory object for importer and exporter objects. As several packages tend
/// to read a file including header as one operation, we need an importer that abstracts this 
/// operation and hides some of the data management, like loading the data before deciding how to
  /// import the data, e.g. as masks or data layers.

// Forward declaration
class LayerIO;
class LayerIOPrivate;
typedef boost::shared_ptr<LayerIOPrivate> LayerIOPrivateHandle;

// Class definition
class LayerIO : public Core::Lockable
{
  CORE_SINGLETON( LayerIO );

  // -- constructor / destructor --
private:
  LayerIO();
  virtual ~LayerIO();

  // -- Importer/Exporter registration --
public:

  /// REGISTER_IMPORTER:
  /// Register an importer that abstracts importing a file in a certain file format
  template < class IMPORTER >
  void register_importer()
  {
    // Generate a new information class
    LayerImporterInfoHandle info( new LayerImporterInfo( 
      LayerImporterBuilderBaseHandle( new LayerImporterBuilder< IMPORTER > ),
      IMPORTER::GetName(), IMPORTER::GetFileTypes(), IMPORTER::GetPriority(),
      IMPORTER::GetType() ) );
  
    // Insert information into internal database
    this->register_importer_internal( info );
  }
  
  /// REGISTER_EXPORTER:
  /// Register an importer that abstracts importing a file in a certain file format
  template < class EXPORTER >
  void register_exporter()
  {
    // Generate a new information class
    LayerExporterInfoHandle info( new LayerExporterInfo( 
      LayerExporterBuilderBaseHandle( new LayerExporterBuilder< EXPORTER > ),
      EXPORTER::GetName(), EXPORTER::GetFileTypes() ) );

    // Insert information into internal database
    this->register_exporter_internal( info );
  }

  // -- internals of registration process --
private:
  /// REGISTER_IMPORTER_INTERNAL
  /// Registration of the importers into the internals of this class
  void register_importer_internal( LayerImporterInfoHandle info );

  /// REGISTER_EXPORTER_INTERNAL
  /// Registration of the exporters into the internals of this class
  void register_exporter_internal( LayerExporterInfoHandle info );

  // -- Get the types of importers available --
public:
  /// GET_SINGLE_FILE_IMPORTER_TYPES
  /// Get the names of all the importers that are available for a single file
  std::vector< std::string > get_single_file_importer_types();

  /// GET_FILE_SERIES_IMPORTER_TYPES
  /// Get the names of all the importers that are available for file series
  std::vector< std::string > get_file_series_importer_types();
  
  /// GET_EXPORTER_TYPES
  /// Get the names of all the exporters that are available
  std::vector< std::string > get_exporter_types();

  // -- functions for creating an importer
public: 
  /// CREATE_SINGLE_FILE_IMPORTER:
  /// This function creates a new importer by checking the file extension and it will return
  /// the appropriate importer. If an importer name is given as well, it will restrain the 
  /// search to that specific name of importer
  bool create_single_file_importer( const std::string& filename, 
    LayerImporterHandle& importer, 
    std::string& error, const std::string& importername = "");
    
  /// CREATE_FILE_SERIES_IMPORTER:  
  /// This function creates a new importer by checking the file extension and it will return
  /// the appropriate importer. If an importer name is given as well, it will restrain the 
  /// search to that specific name of importer
  bool create_file_series_importer( const std::vector<std::string>& filenames, 
    LayerImporterHandle& importer, 
    std::string& error, const std::string& importername = "");
    
  /// CREATE_EXPORTER:
  /// This function creates a new exporter by checking the file extension and it will return
  /// the appropriate exporter. If an exporter name is given as well, it will restrain the 
  /// search to that specific name of exporter            
  bool create_exporter( LayerExporterHandle& exporter, std::vector< LayerHandle >& layers, 
    const std::string importername = "", const std::string extension = "" );
    
  // -- internals --
public:
  LayerIOPrivateHandle private_;

  // -- static functions --
public:
  /// FINDFILESERIES
  /// Process the names/name of a file in a file series and find all the files that should
  /// belong to that file series bsed on the filenames of the files in the same directory.
  static bool FindFileSeries( std::vector<std::string >& filenames );

  static const std::string DATA_MODE_C;
  static const std::string SINGLE_MASK_MODE_C;
  static const std::string LABEL_MASK_MODE_C;
  static const std::string BITPLANE_MASK_MODE_C;
    
};

/// Macro for adding function that registers a new importer
/// Note these functions will be called in the init call of the program.
#define SEG3D_REGISTER_IMPORTER(namesp, name)\
namespace Core\
{\
  using namespace namesp;\
  void register_##name()\
  {\
    LayerIO::Instance()->register_importer<name>();\
  }\
}

/// Macro for adding function that registers a new exporter
/// Note these functions will be called in the init call of the program.
#define SEG3D_REGISTER_EXPORTER(namesp, name)\
  namespace Core\
{\
  using namespace namesp;\
  void register_##name()\
{\
  LayerIO::Instance()->register_exporter<name>();\
}\
}

} // end namespace seg3D

#endif
