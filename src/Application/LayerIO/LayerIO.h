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

// Utils includes
#include <Utils/Core/Singleton.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/LayerIO/LayerImporter.h>
#include <Application/LayerIO/LayerImporterInfo.h>

namespace Seg3D
{
// LAYERIO:
// This class is the factory object for importer and exporter objects. As several packages tend
// to read a file including header as one operation, we need an importer that abstracts this 
// operation and hides some of the data management, like loading the data before deciding how to
// import the data, e.g. as masks or data layers.

// Forward declaration
class LayerIO;

// Class definition
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


  // -- Get the types of importers available --
public:
  // GET_IMPORTER_TYPES:
  // Get the names of all the importers that are available
  importer_types_type get_importer_types();

private:
  // The internal list of importers
  typedef std::vector<LayerImporterInfoHandle> importer_list_type;
  importer_list_type importer_list_;


  // -- Locking interface --
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


  // -- Signals for indicating when a file is imported or exported --
public:
  typedef boost::signals2::signal< void( LayerImporterHandle ) > layerimporter_signal_type;

  // LAYER_IMPORT_START_SIGNAL:
  // Indicates that a layer import will start
  layerimporter_signal_type layer_import_start_signal_;

  // LAYER_IMPORT_END_SIGNAL:
  // Indicates that a layer import has ended
  layerimporter_signal_type layer_import_end_signal_;
  
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
