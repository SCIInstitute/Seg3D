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

#ifndef APPLICATION_LAYERMANAGER_LAYERMANAGER_H
#define APPLICATION_LAYERMANAGER_LAYERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 



// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/State/StateHandler.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/Volume/Volume.h>
#include <Core/Geometry/BBox.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Session/Session.h>

namespace Seg3D
{

// Forward declaration
class LayerManager;

// Class definition
class LayerManager : public Core::StateHandler
{
  CORE_SINGLETON( LayerManager );

  // -- Constructor/Destructor --
private:
  LayerManager();
  virtual ~LayerManager();
  
  // -- Set up StateHandler priority --
public:
  // TODO: Fix this and put this in a macro definition
  // --JS
  virtual int get_session_priority() { return SessionPriority::LAYER_MANAGER_PRIORITY_E; }

public:
  typedef std::list < LayerGroupHandle > group_list_type;

  // -- Accessor Functions --
public:
    // Functions for getting a copy of the Layers and Groups with the proper locking
  // GET_GROUPS:
  // this function copies the groups into the vector that is passed
  void get_groups( std::vector< LayerGroupHandle >& vector_of_groups );
  
  // GET_LAYERS:
  // this function copies the layers into the vector that is passed
  void get_layers( std::vector< LayerHandle >& vector_of_layers );
  
  // GET_LAYERS_IN_GROUP:
  // this function copies the layers into the vector that is passed
  void get_layers_in_group( LayerGroupHandle group, std::vector< LayerHandle > &vector_of_layers);
  
  // CHECK_FOR_SAME_GROUP:
  // this function returns true if the layer id's that are passed are from layer's in the same
  // group
  bool check_for_same_group( const std::string layer_to_insert_id, 
    const std::string layer_below_id );
  
  // GET_LAYER_GROUP:
  // this function returns the group with the id that is passed
  LayerGroupHandle get_layer_group( std::string group_id );
  
  // GET_LAYER_BY_ID:
  // this function returns a handle to the layer with the id that is passed
  LayerHandle get_layer_by_id( const std::string& layer_id );
  
  // GET_LAYER_BY_NAME:
  // this function returns a handle to a layer with the name that is passed
  LayerHandle get_layer_by_name( const std::string& layer_name );

  // GET_DATA_LAYER_BY_ID:
  // this function returns a handle to the layer with the id that is passed
  DataLayerHandle get_data_layer_by_id( const std::string& layer_id );

  // GET_MASK_LAYER_BY_ID:
  // this function returns a handle to the layer with the id that is passed
  MaskLayerHandle get_mask_layer_by_id( const std::string& layer_id );

  // GET_ACTIVE_LAYER:
  // This function returns a handle to the active layer
  LayerHandle get_active_layer();
  
  // GET_LAYER_NAMES:
  // This function returns a vector of layer ID and name pairs of the specified layer type.
  void get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    int type = Core::VolumeType::ALL_E );

  // GET_LAYER_NAMES:
  // This function returns a vector of layer ID and name pairs of the specified layer type.
  void get_layer_names_from_group( LayerGroupHandle group,
    std::vector< LayerIDNamePair >& layer_names, 
    int type = Core::VolumeType::ALL_E );

  // Layer Action Functions
public:
  // INSERT_LAYER:
  // This function returns true when it successfully inserts a layer
  bool insert_layer( LayerHandle layer );
  
  // MOVE_LAYER_ABOVE:
  // this function returns true when it has successfully inserted a layer above another layer
  bool move_layer_above( LayerHandle layer_to_move, LayerHandle target_layer );

  // MOVE_LAYER_BELOW:
  // this function returns true when it has successfully inserted a layer above another layer
  bool move_layer_below(  LayerHandle layer_to_move, LayerHandle target_layer );

  // DELETE_LAYERS:
  // this function deletes the selected layers in the group that is passed
  void delete_layers( LayerGroupHandle group );

  // DELETE_LAYER:
  // this function deletes the layer
  void delete_layer( LayerHandle layer );

  // SET_ACTIVE_LAYER:
  // this function sets the active layer
  void set_active_layer( LayerHandle layer );

  // DELETE_ALL:
  bool delete_all();

private:
  // VALIDATE_LAYER_MOVE:
  // this function is used to validate that a layer move is the kind we want to allow. 
  // Currently the layers have to be the same type to successfully move.
  bool validate_layer_move( LayerHandle layer_above, LayerHandle layer_below );

public:
  // Take an atomic snapshot of visual properties of layers for rendering in the specified viewer
  LayerSceneHandle compose_layer_scene( size_t viewer_id );

  // Get the bounding box of all layers
  Core::BBox get_layers_bbox();
  
  // Group Action Functions
public:
  // MOVE_GROUP_ABOVE:
  // this function handle changing the order of a group in the list
  bool move_group_above( std::string group_to_move_id, std::string group_below_id );

private:
  // INSERT_GROUP:
  // this function handle inserting a group into the list
  void insert_group( LayerGroupHandle group_above, LayerGroupHandle group_below );

  // HANDLE_LAYER_NAME_CHANGED:
  // This function is connected to the value_changed_signal_ of all the layers and relays
  // it to the layer_name_changed_signal_ of LayerManager.
  void handle_layer_name_changed( std::string layer_id, std::string name );

  // Typedef's for the Mutex
public: 
  typedef Core::StateEngine::mutex_type mutex_type;
  typedef Core::StateEngine::lock_type lock_type;
  
  // GET_MUTEX:
  // Get the mutex, so it can be locked by the interface that is built
  // on top of this
  mutex_type& get_mutex();
  
  // state variables
public:
  Core::StateStringHandle active_layer_state_;

public:
  // TODO: There are too many signals in here, we should clean this up
  // --JS

  // -- Signal/Slots --
  typedef boost::signals2::signal< void( LayerHandle ) > layer_signal_type;
  typedef boost::signals2::signal< void( std::string ) > layer_name_signal_type;
  typedef boost::signals2::signal< void( LayerHandle, int ) > layer_at_signal_type;
  typedef boost::signals2::signal< void( LayerGroupHandle ) > group_signal_type;
  typedef boost::signals2::signal< void( std::string, int ) > group_at_signal_type;
  typedef boost::signals2::signal< void( std::vector< LayerHandle > ) > layers_signal_type;
  typedef boost::signals2::signal< void() > layers_changed_signal_type;
  typedef boost::signals2::signal< void() > groups_changed_signal_type;
  
  
  // GROUP_INTERNALS_CHANGED_SIGNAL_:
  // signal is triggered when any of the contents of a group change
  group_signal_type group_internals_changed_signal_;
  
  groups_changed_signal_type groups_changed_signal_;
  
  // ACTIVE_LAYER_CHANGED_SIGNAL:
  // This signal is triggered after the active layer is changed
  layer_signal_type active_layer_changed_signal_; 
    
  // LAYER_INSERTED_SIGNAL:
  // This signal is triggered after a layer has been inserted
  layer_signal_type layer_inserted_signal_;

  // LAYERS_DELETED_SIGNAL:
  // This signal is triggered after layers have been deleted and passes a vector of layerhandles
  layers_signal_type layers_deleted_signal_;
  
  // GROUP_INSERTED_SIGNAL:
  // This signal is triggered when a group has changed its order int the group list
  group_signal_type group_inserted_signal_; 
  
  // GROUP_DELETED_SIGNAL:
  // This signal is triggered when a group has been deleted from the layer manager
  group_signal_type group_deleted_signal_;
  
  // LAYERS_CHANGED_SIGNAL:
  // This gets signaled when layers are inserted, deleted, or reordered.
  layers_changed_signal_type layers_changed_signal_;

  // LAYERS_REORDERED_SIGNAL:
  // Triggered when the order of the layers has been changed.
  layers_changed_signal_type layers_reordered_signal_;

  // LAYER_NAME_CHANGED_SIGNAL:
  // Triggered when the name of a layer has changed.
  // The first parameter is the layer ID, the second is the new name for that layer.
  boost::signals2::signal< void ( std::string, std::string ) > layer_name_changed_signal_;

protected:
  virtual bool pre_save_states( Core::StateIO& state_io );

  // POST_SAVE_STATES:
  // This function is called after the LayerManagers states have been saved and then
  // tells the groups to save their states as well.
  virtual bool post_save_states( Core::StateIO& state_io );

  // POST_LOAD_STATES:
  // this function creates the layers who's information was saved to file, and then tells them
  // to populate their state variables from file
  virtual bool post_load_states( const Core::StateIO& state_io );
  
  // PRE_LOAD_STATES:
  // this function clears out all existing layers before we load a project from file
  virtual bool pre_load_states( const Core::StateIO& state_io );
  
  
private:
    // list of the current groups 
  group_list_type group_list_;
  
  // currently active layer
  LayerHandle active_layer_;

  // -- static functions --
public:
  
  // == functions for validation of an action ==
  
  // CHECKLAYEREXISTANCE:
  // Check whether a layer exists.
  // If it does not exist, the function returns false and an error is string is returned.
  static bool CheckLayerExistance( const std::string& layer_id, std::string& error );
  
  // CHECKLAYEREXISTANCEANDTYPE:
  // Check whether a layer exists and whether it is of the right type.
  // If it does not exist or is not of the right type, the function returns false and
  // an error is string is returned.
  static bool CheckLayerExistanceAndType( const std::string& layer_id, Core::VolumeType type, 
    std::string& error );

  // CHECKLAYERSIZE:
  // Check whether a layer has the right size.
  // If it does not have the right size, the function returns false and an error is string is 
  // returned.  
  static bool CheckLayerSize( const std::string& layer_id1, const std::string& layer_id2,
    std::string& error );
      
  // CHECKLAYERAVAILABILITYFORPROCESSING:
  // Check whether a layer is available for processing, at the end of the filter the data will
  // be replaced with new data. Hence this is write acces.
  // If a layer is not available a notifier is returned that tells can be used to assess when to
  // check for availability again. Even though the notifier may return another process may have
  // grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  // availability. Only when the this function OKs the  
  // NOTE: Availability needs to be tested to ensure that another process is not working on this
  // this layer. 
  static bool CheckLayerAvailabilityForProcessing( const std::string& layer_id, 
    Core::NotifierHandle& notifier );

  // CHECKLAYERAVAILABILITYFORUSE:
  // Check whether a layer is available for use, i.e. data is not changed but needs to remain
  // unchanged during the process. Hence this is read access
  // If a layer is not available a notifier is returned that tells can be used to assess when to
  // check for availability again. Even though the notifier may return another process may have
  // grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  // availability. Only when the this function OKs the  
  // NOTE: Availability needs to be tested to ensure that another process is not working on this
  // this layer. 
  static bool CheckLayerAvailabilityForUse( const std::string& layer_id, 
    Core::NotifierHandle& notifier );
    
  // CHECKLAYERAVAILABILITY:
  // Check whether a layer is available for use. This case processes both of the above cases:
  // if replace is true, it will check for processing (write) access, if it is not replaced, it 
  // will look for use (read) access
  // If a layer is not available a notifier is returned that tells can be used to assess when to
  // check for availability again. Even though the notifier may return another process may have
  // grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  // availability. Only when the this function OKs the  
  // NOTE: Availability needs to be tested to ensure that another process is not working on this
  // this layer. 
  static bool CheckLayerAvailability( const std::string& layer_id, bool replace,
    Core::NotifierHandle& notifier ); 
    
  // == functions for creating and locking layers ==
  
  // These functions can only be called from the application thread
  
  // LOCKFORUSE:
  // Change the layer data_state to IN_USE_C.
  // NOTE: This function can *only* be called from the Application thread.
  static bool LockForUse( LayerHandle layer );
  
  // LOCKFORPROCESSING:
  // Change the layer data_state to PROCESSING_C.
  // NOTE: This function can *only* be called from the Application thread.
  static bool LockForProcessing( LayerHandle layer );
  
  // CREATEANDLOCKMASKLAYER:
  // Create a new mask layer and lock it into the CREATING_C mode.
  // NOTE: This function can *only* be called from the Application thread.
  static bool CreateAndLockMaskLayer( Core::GridTransform transform, const std::string& name, 
    LayerHandle& layer );
  
  // CREATEANDLOCKDATALAYER:
  // Create a new data layer and lock it into the CREATING_C mode.
  // NOTE: This function can *only* be called from the Application thread.
  static bool CreateAndLockDataLayer( Core::GridTransform, const std::string& name,
    LayerHandle& layer );
  
  // == functions for setting data and unlocking layers ==

  // These functions can be called from the filter thread
  
  // DISPATCHUNLOCKLAYER:
  // Change the layer data_state back to available. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchUnlockLayer( LayerHandle layer );

  // DISPATCHDELETELAYER:
  // Delete the layer. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchDeleteLayer( LayerHandle layer );
  
  // DISPATCHUNLOCKORDELETELAYER:
  // Unlock layer if valid, delete otherwise. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchUnlockOrDeleteLayer( LayerHandle layer );

  // DISPATCHINSERTDATAVOLUMEINTOLAYER:
  // Insert a datavolume into a data layer. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchInsertDataVolumeIntoLayer( DataLayerHandle layer, 
    Core::DataVolumeHandle data );

  // DISPATCHINSERTMASKVOLUMEINTOLAYER:
  // Insert a datavolume into a data layer. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchInsertMaskVolumeIntoLayer( MaskLayerHandle layer, 
    Core::MaskVolumeHandle mask );

  // DISPATCHCREATEANDINSERTMASKLAYER:
  // Create and insert the layer. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchCreateAndInsertMaskLayer( std::string name, 
    Core::MaskVolumeHandle mask );
  
  // DISPATCHCREATEANDINSERTMASKLAYER:
  // Create and insert the layer. This function will relay a call to the 
  // Application thread if needed.
  static void DispatchCreateAndInsertDataLayer( std::string name, 
    Core::DataVolumeHandle data );

};

} // end namespace seg3D

#endif

