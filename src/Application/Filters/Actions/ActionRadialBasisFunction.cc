/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Application/Filters/Actions/ActionRadialBasisFunction.h>

#include <Application/Filters/LayerFilter.h>
#include <Application/Filters/Actions/ActionThreshold.h>
#include <Application/Layer/LayerManager.h>

#include <Core/Utils/ConnectionHandler.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Notifier.h>
#include <Core/Utils/Runnable.h>

#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Volume/DataVolume.h>

#include <string>
#include <vector>

// Boost includes
#include <boost/signals2/signal.hpp>
#include <boost/thread/thread.hpp>

// RBF library includes
#include <RBFInterface.h>
#include <vec3.h>

CORE_REGISTER_ACTION( RadialBasisFunction, RadialBasisFunction )

namespace RadialBasisFunction
{

typedef boost::shared_ptr< ActionRadialBasisFunction > ActionRadialBasisFunctionHandle;

using namespace ::Seg3D;
using namespace ::Core;

class NotifierRunnable : public Runnable, public ConnectionHandler
{
public:
  typedef boost::signals2::signal< bool (ActionContextHandle&) > RunSignalType;
  RunSignalType runThresholdSignal_;

  std::string layerID_;
  ActionContextHandle layerContext_;
  NotifierHandle notifier_;
  ActionRadialBasisFunctionHandle action_;

  NotifierRunnable(const std::string& layerID, ActionContextHandle& layerContext, ActionRadialBasisFunctionHandle action) :
    layerID_(layerID),
    layerContext_(layerContext),
    action_(action)
  {
    this->add_connection(
      this->runThresholdSignal_.connect( boost::bind(&ActionRadialBasisFunction::run_threshold, action_, _1) ) );
  }

  ~NotifierRunnable()
  {
    this->disconnect_all();
  }

  virtual void run()
  {
    // wait for layer
    notifier_ = layerContext_->get_resource_notifier();
    notifier_->wait();
    // threshold layer callback
    runThresholdSignal_(layerContext_);
  }
};

class ActionRadialBasisFunctionPrivate
{
public:
  ActionRadialBasisFunctionPrivate() :
    normalOffset_(0),
    thresholdValue_(0) {}

  LayerHandle srcLayer_;
  LayerHandle dstLayer_;
  std::string targetLayerID_;
  VertexList vertices_;
  double normalOffset_;
  std::string kernel_;
  double thresholdValue_;
};

class RadialBasisFunctionAlgo : public LayerFilter
{
public:
  ActionRadialBasisFunctionPrivateHandle actionInternal_;

  RadialBasisFunctionAlgo();
  virtual ~RadialBasisFunctionAlgo();
  
  SCI_BEGIN_RUN()
  {
    DataLayerHandle srcDataLayer = boost::dynamic_pointer_cast<DataLayer>(this->actionInternal_->srcLayer_);
    DataLayerHandle dstDataLayer = boost::dynamic_pointer_cast<DataLayer>(this->actionInternal_->dstLayer_);
    GridTransform srcGridTransform = srcDataLayer->get_grid_transform();

    std::vector<vec3> rbfPointData;
    for (VertexList::iterator it = this->actionInternal_->vertices_.begin();
         it != this->actionInternal_->vertices_.end();
         ++it)
    {
      std::cerr << *it << std::endl;
      rbfPointData.push_back( vec3(it->x(), it->y(), it->z()) );
    }
    // origin and size from source data layer
    Point origin = srcGridTransform.get_origin();
    // TODO: debug print
    std::cerr << "Source data origin: " << origin << std::endl;
    vec3 rbfOrigin(origin.x(), origin.y(), origin.z());
    vec3 rbfGridSize(srcGridTransform.get_nx(), srcGridTransform.get_ny(), srcGridTransform.get_nz());
    vec3 rbfGridSpacing(srcGridTransform.spacing_x(), srcGridTransform.spacing_y(), srcGridTransform.spacing_z());
    // TODO: debug print
    std::cerr << "Source data size: " << rbfGridSize[0] << ", "
                                      << rbfGridSize[1] << ", "
                                      << rbfGridSize[2] << std::endl;

    // From RBF class. ThinPlate is the default kernel.
    Kernel kernel = ThinPlate;
    if (this->actionInternal_->kernel_ == "gaussian")
    {
      kernel = Gaussian;
    }
    else if (this->actionInternal_->kernel_ == "multi_quadratic")
    {
      kernel = MultiQuadratic;
    }

    RBFInterface rbfAlgo(rbfPointData, rbfOrigin, rbfGridSize, rbfGridSpacing, this->actionInternal_->normalOffset_, kernel);
    this->actionInternal_->thresholdValue_ = rbfAlgo.getThresholdValue();

    Core::DataBlockHandle dstDataBlock = Core::StdDataBlock::New( srcGridTransform, Core::DataType::FLOAT_E );
    if ( ! dstDataBlock )
    {
      this->report_error( "Could not allocate enough memory." );
      return;
    }

    for (size_t i = 0; i < dstDataBlock->get_nx(); ++i)
    {
      for (size_t j = 0; j < dstDataBlock->get_ny(); ++j)
      {
        for (size_t k = 0; k < dstDataBlock->get_nz(); ++k)
        {
          dstDataBlock->set_data_at( i, j, k, rbfAlgo.value[i][j][k] );
        }
      }
    }
    dstDataBlock->update_histogram();
    std::cerr << "Min: " << dstDataBlock->get_min() << ", max: " << dstDataBlock->get_max() << std::endl;

    // TODO: threshold from 0 to dataset max to get mask layer

    this->dispatch_insert_data_volume_into_layer(
      this->actionInternal_->dstLayer_,
      Core::DataVolumeHandle(new Core::DataVolume( this->actionInternal_->dstLayer_->get_grid_transform(), dstDataBlock ) ),
      true );
  }
  SCI_END_RUN()

  // GET_FITLER_NAME:
  // The name of the filter, this information is used for generating new layer labels.
  virtual std::string get_filter_name() const
  {
    return "RadialBasisFunction Tool";
  }
  
  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return "RadialBasisFunction";	
  }	
};

RadialBasisFunctionAlgo::RadialBasisFunctionAlgo()
{
}

RadialBasisFunctionAlgo::~RadialBasisFunctionAlgo()
{
}

ActionRadialBasisFunction::ActionRadialBasisFunction() :
  private_( new ActionRadialBasisFunctionPrivate )

{
  this->add_layer_id( this->private_->targetLayerID_ );
  this->add_parameter( this->private_->vertices_ );
  this->add_parameter( this->private_->normalOffset_ );
  this->add_parameter( this->private_->kernel_ );
  this->add_parameter( this->sandbox_ );
}

bool ActionRadialBasisFunction::validate( ActionContextHandle& context )
{
  if (this->private_->vertices_.size() < 3)
  {
    context->report_error("At least 3 points are needed.");
    return false;
  }

  if ( (this->private_->kernel_ != "thin_plate") &&
       (this->private_->kernel_ != "gaussian") &&
       (this->private_->kernel_ != "multi_quadratic") )
  {
    context->report_error("Kernel must be thin_plate or gaussian or multi_quadratic.");
    return false;
  }

  // let's just not allow negative or too small values for now
  if ( this->private_->normalOffset_ < 1.0 )
  {
    context->report_error("Normal offset must be positive and at least 1.0.");
    return false;
  }

  return true;
}

bool ActionRadialBasisFunction::run( ActionContextHandle& context, ActionResultHandle& result )
{
  boost::shared_ptr< RadialBasisFunctionAlgo > algo( new RadialBasisFunctionAlgo() );

  // Set up parameters
  algo->set_sandbox( this->sandbox_ );
  algo->actionInternal_ = this->private_;

  // Find the handle to the layer
  if ( !( algo->find_layer( this->private_->targetLayerID_, this->private_->srcLayer_ ) ) )
  {
    return false;
  }

  // Lock the src layer, so it cannot be used else where
  algo->lock_for_use( this->private_->srcLayer_ );

  // Create the destination layer, which will show progress
  algo->create_and_lock_data_layer_from_layer( this->private_->srcLayer_, private_->dstLayer_ );

  // Return the id of the destination layer.
  result = ActionResultHandle( new ActionResult( this->private_->dstLayer_->get_layer_id() ) );

  // If the action is run from a script (provenance is a special case of script),
  // return a notifier that the script engine can wait on.
  if ( context->source() == ActionSource::SCRIPT_E ||
       context->source() == ActionSource::PROVENANCE_E )
  {
    context->report_need_resource( algo->get_notifier() );
  }

  // Build the undo-redo record
  algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

  // Start the filter.
  Runnable::Start( algo );

  ActionContextHandle layerContext( new ActionContext() );
  // wait for layer
  if ( ! LayerManager::CheckLayerAvailabilityForUse( this->private_->dstLayer_->get_layer_id(), layerContext, this->sandbox_ ) )
  {
    NotifierRunnableHandle notifierThread_(
      new NotifierRunnable( this->private_->dstLayer_->get_layer_id(),
                            layerContext,
                            boost::dynamic_pointer_cast< ActionRadialBasisFunction >(this->shared_from_this()) ) );
      Runnable::Start( notifierThread_ );
  }

	return true;
}

bool ActionRadialBasisFunction::run_threshold( ActionContextHandle& context )
{
  DataLayerHandle dstDataLayer = boost::dynamic_pointer_cast<DataLayer>( this->private_->dstLayer_ );
  double dstMaxValue = dstDataLayer->get_data_volume()->get_data_block()->get_max();
  // TODO: debug print
  std::cerr << "Threshold " << dstDataLayer->get_layer_id() << " min=" << this->private_->thresholdValue_ << ", max=" << dstMaxValue << std::endl;

  ActionThreshold::Dispatch(context,
                            dstDataLayer->get_layer_id(),
                            this->private_->thresholdValue_,
                            dstMaxValue);

  return true;
}

void ActionRadialBasisFunction::Dispatch(
                                           ActionContextHandle context,
                                           const std::string& target,
                                           const VertexList& vertices,
                                           double normalOffset,
                                           const std::string& kernel
                                         )
{
  ActionRadialBasisFunction* action = new ActionRadialBasisFunction;
  action->private_->targetLayerID_ = target;
  action->private_->vertices_ = vertices;
  action->private_->normalOffset_ = normalOffset;
  action->private_->kernel_ = kernel;

  ActionDispatcher::PostAction( ActionHandle( action ), context );
}

}
