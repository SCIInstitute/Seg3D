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

#ifndef APPLICATION_LAYERMANAGER_LAYERIO_H
#define APPLICATION_LAYERMANAGER_LAYERIO_H

// Utils includes
#include <Utils/Singleton/Singleton.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerIO/LayerImporter.h>


namespace Seg3D
{

// ---- Auxilary Classes -----
// LAYERIMPORTER:
// This is the base class for the class that builds the importer objects

class LayerImporterBuilderBase;
typedef boost::shared_ptr<LayerImporterBuilderBase> LayerImporterBuilderBaseHandle;

class LayerImporterBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~LayerImporterBuilderBase()
  {
  }
  
  // the function call to build the object
  virtual LayerImporterHandle build( const std::string& filename ) = 0;
};


// LAYERIMPORTERBUILDER:
// Auxillary class to build the layer importer class

template <class LAYERIMPORTER>
class LayerImporterBuilder: public LayerImporterBuilderBase
{
public:
  // ensure we can delete the builder correctly
  virtual ~LayerImporterBuilder<LAYERIMPORTER>()
  {}

  // The actual builder call
  virtual LayerImporterHandle build( const std::string& filename )
  { return LayerImporterHandle( new LAYERIMPORTER( filename ));}
};

// LAYERIMPORTERINFO
// Class that records the information of the different importers that are available in
// the program

class LayerImporterInfo;
typedef boost::shared_ptr<LayerImporterInfo> LayerImporterInfoHandle;


class LayerImporterInfo 
{
  // -- constructor/destructor --
public:
  LayerImporterInfo( LayerImporterBuilderBaseHandle builder,
    const std::string name, 
    const std::string file_type_string,
    const unsigned int priority );
  
  ~LayerImporterInfo();

  // BUILD:
  // Build the importer
  LayerImporterHandle build( const std::string& filename ) const;

  // NAME:
  // Get the name of the importer
  std::string name() const;

  // CONVERTS_FILE_TYPE:
  // Check whether this importer deals with a specific file type
  bool converts_file_type( const std::string& file_type ) const;
  
  // FILE_TYPES:
  std::string file_type_string() const;
  
  // PRIORITY:
  // Get the priority of the importer
  unsigned int priority() const;

private:
  // Object that knows how to build the importer
  LayerImporterBuilderBaseHandle builder_;
  
  // Name of the importer
  std::string name_;
  
  // The file types the importer will handle
  std::vector<std::string> file_types_; 
  
  // String with name and file types, to be used as a filter in a file selector
  std::string file_type_string_;
  
  // Any file type, e.g. dicom readers tend to use no extension
  bool any_type_;
  
  // The priority of thid importer
  unsigned int priority_;
};


// ------------------------------

// LAYERIO:
// This class is the factory object for importer and exporter objects. As several packages tend
// to read a file including header as one operation, we need an importer that abstracts this 
// operation and hides some of the data management, like loading the data before deciding how to
// import the data, e.g. as masks or data layers.


// Forward declaration
class LayerIO;

class LayerIO : public Utils::Singleton<LayerIO>
{
  // -- typedefs --
public:
  typedef boost::mutex mutex_type;
  typedef boost::unique_lock<mutex_type> lock_type;
  typedef std::vector<std::string> importer_types_type;

  // -- constructor / destructor --
private:
  friend class Utils::Singleton<LayerIO>;
  LayerIO();
  virtual ~LayerIO();

  // -- Importer/Exporter registration --
public:

  // REGISTER_IMPORTER:
  // Register an importer that abstracts importing a file in a certain file format

  template <class IMPORTER>
  void register_importer()
  {
    // Lock the factory
    lock_type lock( mutex_ );

    // Generate a new information class
    LayerImporterInfoHandle info( new LayerImporterInfo( 
      LayerImporterBuilderBaseHandle( new LayerImporterBuilder<IMPORTER> ),
      IMPORTER::Name(), IMPORTER::FileTypes(), IMPORTER::Priority() ) );

    // Insert the information block into the importer list
    importer_list_.push_back( info );
  }

  // GET_IMPORTER_TYPES:
  // Get the names of all the importers that are available
  importer_types_type get_importer_types();

private:
  // The internal list of importers
  typedef std::vector<LayerImporterInfoHandle> importer_list_type;
  importer_list_type importer_list_;

  // -- locking interface --
public:

  // GET_MUTEX:
  // Get the mutex so we can lock it from outside the class
  mutex_type& get_mutex() { return mutex_; } 
  
private:

  // Mutex protecting this resource
  mutex_type mutex_;
  
  // -- functions for creating an importer
public: 

  // CREATE_IMPORTER:
  // This function creates a new importer by checking the file extension and it will return
  // the approriate importer. If an importer name is given as well, it will restrain the 
  // search to that specific name of importer
  bool create_importer( const std::string& filename, LayerImporterHandle& importer,
              const std::string importername = "");

};


// Macro for adding function that registers a new importer
// Note these functions will be called in the init call of the program.

#define SCI_REGISTER_IMPORTER(name)\
void register_importer_##name()\
{\
  LayerIO::Instance()->register_importer<name>();\
} 

} // end namespace seg3D

#endif
