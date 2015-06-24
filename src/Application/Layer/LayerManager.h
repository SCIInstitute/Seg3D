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

#ifndef APPLICATION_LAYER_LAYERMANAGER_H
#define APPLICATION_LAYER_LAYERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>

// Core includes
#include <Core/State/StateHandler.h>
#include <Core/Utils/Singleton.h>
#include <Core/Volume/Volume.h>
#include <Core/Geometry/BBox.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/Layer/DataLayer.h>
#include <Application/Layer/MaskLayer.h>
#include <Application/Layer/LargeVolumeLayer.h>
#include <Application/Project/Project.h>

namespace Seg3D
{

// Forward declaration
class LayerManager;
class LayerManagerPrivate;
typedef boost::shared_ptr< LayerManagerPrivate > LayerManagerPrivateHandle;

// Sandbox
typedef long long SandboxID;

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

  // -- Accessor Functions --
public:
  // Functions for getting a copy of the Layers and Groups with the proper locking
  /// GET_GROUPS:
  /// this function copies the groups into the vector that is passed
  void get_groups( std::vector< LayerGroupHandle >& groups );
  
  /// GET_LAYERS:
  /// Get all layers in top to bottom order.
  void get_layers( std::vector< LayerHandle >& layers );
    
  /// FIND_GROUP:
  /// this function returns the group with the id that is passed.
  LayerGroupHandle find_group( const std::string& group_id );

  /// FIND_GROUP:
  /// this function returns the group with the provenance id that is passed.
  LayerGroupHandle find_group( ProvenanceID provenance_id );
  
  /// FIND_LAYER_BY_ID:
  /// Find the layer with the given ID.
  /// If a sandbox number is given, it searches in that sandbox instead.
  LayerHandle find_layer_by_id( const std::string& layer_id, SandboxID sandbox = -1 );
  
  /// FIND_LAYER_BY_NAME:
  /// This function returns a handle to a layer with the name that is passed.
  /// If a sandbox number is given, it searches in that sandbox instead.
  LayerHandle find_layer_by_name( const std::string& layer_name, SandboxID sandbox = -1 );

  /// GET_LAYER_BY_PROVENANCE_ID:
  /// This function returns a handle to a layer with the provenance id that was given.
  /// If a sandbox number is given, it searches in that sandbox instead.
  LayerHandle find_layer_by_provenance_id( ProvenanceID provenance_id, SandboxID sandbox = -1 );

  /// GET_DATA_LAYER_BY_ID:
  /// this function returns a handle to the layer with the id that is passed.
  /// If a sandbox number is given, it searches in that sandbox instead.
  DataLayerHandle find_data_layer_by_id( const std::string& layer_id, SandboxID sandbox = -1 );

  /// GET_MASK_LAYER_BY_ID:
  /// this function returns a handle to the layer with the id that is passed.
  /// If a sandbox number is given, it searches in that sandbox instead.
  MaskLayerHandle find_mask_layer_by_id( const std::string& layer_id, SandboxID sandbox = -1 );

  /// GET_ACTIVE_LAYER:
  /// This function returns a handle to the active layer
  /// Locks: StateEngine
  LayerHandle get_active_layer();
  
  /// GET_LAYER_NAMES:
  /// This function returns a vector of layer ID and name pairs of the specified layer type.
  void get_layer_names( std::vector< LayerIDNamePair >& layer_names, 
    int type = Core::VolumeType::ALL_E );

  /// GET_GROUP_POSITION:
  /// Returns the position of the given group.
  size_t get_group_position( LayerGroupHandle group );

  /// IS_SANDBOX:
  /// Check whether the given sandbox ID is valid.
  /// NOTE: -1 is considered valid because it represents the normal context.
  bool is_sandbox( SandboxID sandbox_id );

  // Layer Action Functions
private:
  friend class ActionImportLargeVolumeLayer;
  friend class ActionImportLayer;
  friend class ActionImportSeries;
  friend class ActionNewMaskLayer;
  friend class ActionDuplicateLayer;
  friend class ActionMoveLayer;
  friend class ActionDeleteLayers;
  friend class ActionShiftActiveLayer;
  friend class ActionActivateLayer;
  friend class ActionMoveGroup;
  friend class ActionCreateSandbox;
  friend class ActionDeleteSandbox;
  friend class ActionCopyLayerIntoSandbox;
  friend class ActionMigrateSandboxLayer;
  friend class ActionRecreateLayer;
  friend class LayerUndoBufferItem;
  friend class LayerRecreationUndoBufferItem;

  /// INSERT_LAYER:
  /// This function returns true when it successfully inserts a layer
  bool insert_layer( LayerHandle layer, SandboxID sandbox = -1 );

  /// MOVE_LAYER:
  /// Move the src_layer to the position right above the dst_layer.
  /// If dst_layer is not given, then move src_layer to the bottom of its group.
  /// Returns true on success, otherwise false.
  /// NOTE: A NOP is considered successful.
  bool move_layer( LayerHandle src_layer, LayerHandle dst_layer = LayerHandle() );

  /// DELETE_LAYERS:
  /// Delete the given layers.
  void delete_layers( const std::vector< LayerHandle >& layers, SandboxID sandbox = -1 );

  /// DELETE_LAYER:
  /// Delete the given layer.
  void delete_layer( LayerHandle layer, SandboxID sandbox = -1 );

  /// SET_ACTIVE_LAYER:
  /// Sets the active layer.
  void set_active_layer( LayerHandle layer );

  /// SHIFT_ACTIVE_LAYER:
  /// Set the active layer to the one above or below the current active layer.
  void shift_active_layer( bool downward = false );

  /// MOVE_GROUP:
  /// Move src_group to the position right above the dst_group.
  /// Returns true on success, otherwise false.
  bool move_group( LayerGroupHandle src_group, LayerGroupHandle dst_group );

  /// UNDELETE_LAYERS:
  /// Add the deleted layers back.
  /// The first parameter contains a vector of the layers, the second parameter contains
  /// the original position of the group that contained the layer, the third parameter
  /// contains the original position of the layer within its group.
  /// NOTE: The three parameters must have the same number of elements.
  void undelete_layers( const std::vector< LayerHandle >& layers, 
    const std::vector< size_t >& group_pos, const std::vector< size_t >& layer_pos );

  /// CREATE_SANDBOX:
  /// Create a sandbox and return the sandbox ID.
  /// The returned ID starts from 1 and increases. 
  /// NOTE: Sandbox 0 is reserved for provenance playback.
  SandboxID create_sandbox();

  /// CREATE_SANDBOX:
  /// Try to create a sandbox with the specified ID. 
  /// Fails if the sandbox ID is negative, or if the sandbox already exists.
  /// Returns true on success, otherwise false.
  bool create_sandbox( SandboxID sandbox );

  /// DELETE_SANDBOX:
  /// Delete a sandbox.
  /// Returns true if the sandbox was found and deleted, otherwise false.
  bool delete_sandbox( SandboxID sandbox );

public:
  /// Take an atomic snapshot of visual properties of layers for rendering in the specified viewer
  LayerSceneHandle compose_layer_scene( size_t viewer_id );

  /// Get the bounding box of all layers
  Core::BBox get_layers_bbox();

  // -- locking --
public: 
  typedef Core::StateEngine::mutex_type mutex_type;
  typedef Core::StateEngine::lock_type lock_type;
  
  /// GET_MUTEX:
  /// Get the mutex, so it can be locked by the interface that is built
  /// on top of this
  mutex_type& get_mutex();
  
  // state variables
public:
  Core::StateLabeledOptionHandle active_layer_state_;

  // -- Signal --
public:
  /// LAYER_INSERTED_SIGNAL:
  /// Triggered after a layer has been inserted.
  /// The first parameter is the new layer. The second parameter indicates whether
  /// a new group has been created.
  boost::signals2::signal< void ( LayerHandle, bool ) > layer_inserted_signal_;

  /// LAYERS_DELETED_SIGNAL:
  /// Triggered after layers have been deleted.
  /// The first parameter is a vector of deleted layer IDs.
  /// The second parameter is a vector of group IDs from 
  /// which the layers have been deleted.
  /// The third parameter indicates whether any group has been deleted.
  boost::signals2::signal< void ( std::vector< std::string >, 
    std::vector< std::string >, bool ) > layers_deleted_signal_;

  /// LAYERS_REORDERED_SIGNAL:
  /// Triggered when the order of the layers has been changed.
  /// The parameter is the ID of the group whose layers have been reordered.
  boost::signals2::signal< void ( std::string ) > layers_reordered_signal_;

  /// GROUPS_REORDERED_SIGNAL:
  /// Triggered when layer groups have been reordered.
  boost::signals2::signal< void () > groups_reordered_signal_;

  /// LAYERS_CHANGED_SIGNAL:
  /// Triggered when layers are inserted, deleted, or reordered. 
  /// It is a combination of layer_inserted_signal_, layers_deleted_signal_,
  /// layers_reordered_signal_, and groups_reordered_signal_.
  /// NOTE: It is always triggered after the individual ones.
  boost::signals2::signal< void() > layers_changed_signal_;

  /// MASK_LAYER_ISOSURFACE_CREATED_SIGNAL:
  /// Triggered when isosurface is created.
  boost::signals2::signal< void() > mask_layer_isosurface_created_signal_;

  /// MASK_LAYER_ISOSURFACE_DELETED_SIGNAL:
  /// Triggered when isosurface is deleted.
  boost::signals2::signal< void() > mask_layer_isosurface_deleted_signal_;

  /// ACTIVE_LAYER_CHANGED_SIGNAL:
  /// This signal is triggered after the active layer is changed
  boost::signals2::signal< void ( LayerHandle ) > active_layer_changed_signal_; 

  /// LAYER_NAME_CHANGED_SIGNAL:
  /// Triggered when the name of a layer has changed.
  /// The first parameter is the layer ID, the second is the new name for that layer.
  boost::signals2::signal< void ( std::string, std::string ) > layer_name_changed_signal_;

  /// LAYER_VOLUME_CHANGED_SIGNAL:
  /// Triggered when the volume of a layer has changed.
  /// The first parameter is the layer handle.
  boost::signals2::signal< void ( LayerHandle ) > layer_volume_changed_signal_;
  
  /// LAYER_DATA_CHANGED_SIGNAL:
  /// Triggered when the layer data state is changed. This tracks whether layers
  /// are being locked for processing and when new data will be available
  boost::signals2::signal< void ( LayerHandle ) > layer_data_changed_signal_;

  /// SANDBOX_CREATED_SIGNAL_:
  /// Triggered when a sandbox has been created.
  boost::signals2::signal< void ( SandboxID ) > sandbox_created_signal_;

  /// SANDBOX_DELETED_SIGNAL_:
  /// Triggered when a sandbox has been deleted.
  boost::signals2::signal< void ( SandboxID ) > sandbox_deleted_signal_;

  // -- Scripting related signals --
public:
  /// SCRIPT_BEGIN_SIGNAL:
  /// Indicate the beginning of a script.
  /// The first parameter is sandbox in which the script is running, 
  /// the second parameter is the the script name.
  /// NOTE: The sandbox can be used to uniquely identify a running script.
  boost::signals2::signal< void ( SandboxID, std::string ) > script_begin_signal_;

  /// SCRIPT_END_SIGNAL:
  /// Indicate the end of a script.
  /// The parameter is the sandbox in which the script is running.
  boost::signals2::signal< void ( SandboxID ) > script_end_signal_;

  /// SCRIPT_PROGRESS_SIGNAL:
  /// Report the progress of a script.
  /// The first parameter is the sandbox in which the script is running.
  /// The second parameter is the name of the current running step.
  /// The third parameter is the number of steps that have finished.
  /// The fourth parameter is the total number of steps.
  boost::signals2::signal< void ( SandboxID, std::string, size_t, size_t ) > script_progress_signal_;

protected:
  /// PRE_SAVE_STATES:
  /// This function is called before the LayerManager's states are being saved
  virtual bool pre_save_states( Core::StateIO& state_io );

  /// POST_SAVE_STATES:
  /// This function is called after the LayerManager's states have been saved and then
  /// tells the groups to save their states as well.
  virtual bool post_save_states( Core::StateIO& state_io );

  /// POST_LOAD_STATES:
  /// this function creates the layers who's information was saved to file, and then tells them
  /// to populate their state variables from file
  virtual bool post_load_states( const Core::StateIO& state_io );
  
  /// PRE_LOAD_STATES:
  /// this function clears out all existing layers before we load a project from file
  virtual bool pre_load_states( const Core::StateIO& state_io );
  
private:
  friend class LayerManagerPrivate;
  LayerManagerPrivateHandle private_;

  // -- static functions --
public:
  
  // == functions for validation of an action ==
  
  /// FINDLAYER:
  /// Find a layer inside the layer manager
  static LayerHandle FindLayer( const std::string& layer_id, SandboxID sandbox = -1 );

  /// FINDLAYER:
  /// Find a layer inside the layer manager
  static LayerHandle FindLayer( ProvenanceID prov_id, SandboxID sandbox = -1 );

  /// FINDGROUP:
  /// Find a layer inside the layer manager
  static LayerGroupHandle FindGroup( const std::string& group_id );

  /// FINDGROUP:
  /// Find a layer inside the layer manager
  static LayerGroupHandle FindGroup( ProvenanceID prov_id );
  
  /// FINDMASKLAYER:
  /// Find a mask layer inside the layer manager
  static MaskLayerHandle FindMaskLayer( const std::string& layer_id, SandboxID sandbox = -1 );

  /// FINDDATALAYER:
  /// Find a data layer inside the layer manager
  static DataLayerHandle FindDataLayer( const std::string& layer_id, SandboxID sandbox = -1 );

//// Functions that need to be removed ///////////

  /// CHECKGROUPEXISTENCE:
  /// Check whether a group exists.
  /// If it does not exist, the function returns false and an error is string is returned.
  /// TODO: Need to make this one obsolete for provenance reasons everything will have to be
  /// done with layerid lists
  /// --JGS
  static bool CheckGroupExistence( const std::string& layer_id, std::string& error );
//////////////////////////////////////////////////

  /// CHECKSANDBOXEXISTENCE:
  /// Check whether a sandbox exists.
  /// If it does not exist, the function returns false and reports the error in the context .
  static bool CheckSandboxExistence( SandboxID sandbox, Core::ActionContextHandle context );

  /// CHECKLAYEREXISTENCE:
  /// Check whether a layer exists.
  /// If it does not exist, the function returns and reports the error in the context 
  static bool CheckLayerExistence( const std::string& layer_id, 
    Core::ActionContextHandle context, SandboxID sandbox = -1 );
  
  /// Check whether a layer exists.
  /// If it does not exist, the function returns false.
  static bool CheckLayerExistence( const std::string& layer_id, SandboxID sandbox = -1 ); 
  
  /// CHECKLAYEREXISTENCEANDTYPE:
  /// Check whether a layer exists and whether it is of the right type.
  /// If it does not exist or is not of the right type, the function returns the error in the
  /// context.
  static bool CheckLayerExistenceAndType( const std::string& layer_id, Core::VolumeType type, 
    Core::ActionContextHandle context, SandboxID sandbox = -1 );

  /// CHECKLAYERSIZE:
  /// Check whether a layer has the right size.
  /// If it does not have the right size, the function returns false and returns the error in 
  /// the context.  
  static bool CheckLayerSize( const std::string& layer_id1, const std::string& layer_id2,
    Core::ActionContextHandle context, SandboxID sandbox = -1 );
      
  /// CHECKLAYERAVAILABILITYFORPROCESSING:
  /// Check whether a layer is available for processing, at the end of the filter the data will
  /// be replaced with new data. Hence this is write access.
  /// If a layer is not available a notifier is returned that tells can be used to assess when to
  /// check for availability again. Even though the notifier may return another process may have
  /// grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  /// availability. 
  /// NOTE: Availability needs to be tested to ensure that another process is not working on this
  /// this layer. 
  static bool CheckLayerAvailabilityForProcessing( const std::string& layer_id, 
    Core::ActionContextHandle context, SandboxID sandbox = -1 );

  /// CHECKLAYERAVAILABILITYFORUSE:
  /// Check whether a layer is available for use, i.e. data is not changed but needs to remain
  /// unchanged during the process. Hence this is read access
  /// If a layer is not available a notifier is returned that tells can be used to assess when to
  /// check for availability again. Even though the notifier may return another process may have
  /// grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  /// availability. 
  /// NOTE: Availability needs to be tested to ensure that another process is not working on this
  /// this layer. 
  static bool CheckLayerAvailabilityForUse( const std::string& layer_id, 
    Core::ActionContextHandle context, SandboxID sandbox = -1 );
    
  /// CHECKLAYERAVAILABILITY:
  /// Check whether a layer is available for use. This case processes both of the above cases:
  /// if replace is true, it will check for processing (write) access, if it is not replaced, it 
  /// will look for use (read) access
  /// If a layer is not available a notifier is returned that tells can be used to assess when to
  /// check for availability again. Even though the notifier may return another process may have
  /// grabbed it in the mean time. In that case a new notifier will need to be issued by rechecking
  /// availability.
  /// NOTE: Availability needs to be tested to ensure that another process is not working on this
  /// this layer. 
  static bool CheckLayerAvailability( const std::string& layer_id, bool replace,
    Core::ActionContextHandle context, SandboxID sandbox = -1 );  
    
  // == functions for creating and locking layers ==
public: 
  /// KEY_TYPE:
  /// When locking a layer a key is returned. This key keeps track of the asynchronous process
  /// and is needed to reinsert a volume into layer. The purpose of the key is to ensure that
  /// a volume is only inserted by the asynchronous process if the key matches the one generated
  /// when the layer was locked.
  typedef Layer::filter_key_type filter_key_type;

  // These functions can only be called from the application thread
  
  /// LOCKFORUSE:
  /// Change the layer data_state to IN_USE_C.
  /// NOTE: This function can *only* be called from the Application thread.
  static bool LockForUse( LayerHandle layer, filter_key_type key = filter_key_type( 0 ) );
  
  /// LOCKFORPROCESSING:
  /// Change the layer data_state to PROCESSING_C.
  /// NOTE: This function can *only* be called from the Application thread.
  static bool LockForProcessing( LayerHandle layer, filter_key_type key = filter_key_type( 0 ) );
  
  /// CREATEANDLOCKMASKLAYER:
  /// Create a new mask layer and lock it into the CREATING_C mode.
  /// NOTE: This function can *only* be called from the Application thread.
  static bool CreateAndLockMaskLayer( Core::GridTransform transform, const std::string& name, 
    LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key = filter_key_type( 0 ), 
    SandboxID sandbox = -1 );
  
  /// CREATEANDLOCKDATALAYER:
  /// Create a new data layer and lock it into the CREATING_C mode.
  /// NOTE: This function can *only* be called from the Application thread.
  static bool CreateAndLockDataLayer( Core::GridTransform, const std::string& name,
    LayerHandle& layer, const LayerMetaData& meta_data, filter_key_type key = filter_key_type( 0 ), 
    SandboxID sandbox = -1 );

  /// Create a cropped version of a large volume.
  /// NOTE: This function can *only* be called from the Application thread.
  static bool CreateCroppedLargeVolumeLayer( Core::LargeVolumeSchemaHandle schema,
    const Core::GridTransform& crop_trans, const std::string& name,
    LayerHandle& layer, const LayerMetaData& meta_data, SandboxID sandbox = -1 );
  
  // == functions for setting data and unlocking layers ==

  // These functions can be called from the filter thread
  
  /// DISPATCHUNLOCKLAYER:
  /// Change the layer data_state back to available. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchUnlockLayer( LayerHandle layer, filter_key_type key = filter_key_type( 0 ),
    SandboxID sandbox = -1 );

  /// DISPATCHDELETELAYER:
  /// Delete the layer. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchDeleteLayer( LayerHandle layer, filter_key_type key = filter_key_type( 0 ),
    SandboxID sandbox = -1 );
  
  /// DISPATCHUNLOCKORDELETELAYER:
  /// Unlock layer if valid, delete otherwise. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchUnlockOrDeleteLayer( LayerHandle layer, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTDATAVOLUMEINTOLAYER:
  /// Insert a data volume into a data layer. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchInsertDataVolumeIntoLayer( DataLayerHandle layer, 
    Core::DataVolumeHandle data, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTMASKVOLUMEINTOLAYER:
  /// Insert a mask volume into a mask layer. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchInsertMaskVolumeIntoLayer( MaskLayerHandle layer, 
    Core::MaskVolumeHandle mask, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTVOLUMEINTOLAYER:
  /// Insert a mask or data volume into a layer. This function will relay a call to the 
  /// Application thread if needed.
  static void DispatchInsertVolumeIntoLayer( LayerHandle layer, 
    Core::VolumeHandle mask, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTDATASLICEINTOLAYER:
  /// Insert a data slice into a data layer. 
  static void DispatchInsertDataSliceIntoLayer( DataLayerHandle layer,
    Core::DataSliceHandle data, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTDATASLICEINTOLAYER:
  /// Insert a data slice into a data layer. 
  static void DispatchInsertDataSlicesIntoLayer( DataLayerHandle layer,
    std::vector<Core::DataSliceHandle> data, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTMASKSLICEINTOLAYER:
  /// Insert a data slice into a data layer. 
  static void DispatchInsertMaskSliceIntoLayer( MaskLayerHandle layer,
    Core::MaskDataSliceHandle mask, ProvenanceID provid,
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  /// DISPATCHINSERTMASKSLICESINTOLAYER:
  /// Insert a data slice into a data layer. 
  static void DispatchInsertMaskSlicesIntoLayer( MaskLayerHandle layer,
    std::vector<Core::MaskDataSliceHandle> mask, ProvenanceID provid, 
    filter_key_type key = filter_key_type( 0 ), SandboxID sandbox = -1 );

  // -- functions for obtaining the current layer and group id counters --
  typedef std::vector<int> id_count_type;
  
  /// GETLAYERIDCOUNT:
  /// Get the current count of the group and layer ids
  static id_count_type GetLayerIdCount();

  /// GETLAYERINVALIDIDCOUNT:
  /// Get a default id count that has no valid ids.
  static id_count_type GetLayerInvalidIdCount();
  
  /// SETLAYERIDCOUNT:
  /// Set the current count of group and layer
  /// NOTE: This function should only be called by the undo buffer
  static void SetLayerIdCount( id_count_type id_count );

};

} // end namespace seg3D

#endif
