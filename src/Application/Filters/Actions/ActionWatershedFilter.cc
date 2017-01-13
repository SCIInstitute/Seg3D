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
 
 
 
 /*
  * Version 0.1
  * 22 / 06 / 2016
  * 
  * Uses ITK's Watershedding filter to segment 2D and 3D data layers.  Never uses 0 as
  * a label so as to numerically discriminate from nonsegmented regions.
  * 
  * TODO:
  * 	- Speed up relabeling.  I included timing in the console output (can be switched
  * 		with the _ConsoleOuput define).  See timing for low threshold/level values.  
  * 		Can be an extremely long wait; especially for 3D watersheds.
  * 	- Break segmentations into useful objects (masks, rgb, etc) it is currently
  * 		saved as a WtrLabelType label (see below register action).
  * 			~ Currently uses a for loop of unsigned int's to designate N unique labels.  This
  * 				is then outputted as a single grayscale layer.  Simply implementing RGB data
  * 				types in Seg3D would significantly improve the variety of information conveyed
  * 				visually.  I have mock code showing how easy it would be to do when Seg3D handles
  * 				RGB data types.  Note that a label (grayscale value) of 0 is never used.  Similarly, 
  * 			 	a RGB value of (0, 0, 0) is never used in the mock code.  This allows for a hook to
  * 				identify unsegmented regions.
  * 			~ Masks would potentially enable quick sub-segmentation of undersegmented regions.
  * 				Although even 255 individual mask layers seems too much for a human to
  * 				efficiently deal with.  Perhaps an option to output mask layers grouped
  * 				by defined pixel area bins?  Or even use a gausian fit of the pixel area to
  * 				automatically size n bins.
  * 	- Allow for thresholding of certain sized/grouped objects using connected component along
  * 		with a gemoetrical sizing algorithm (could be as simple as pixel area).  See above about
  * 		gausian fit.
  * 	- Further investigate the accuracy of 3D watershedding.  I ran a heart MRI data layer through
  * 		the filter and was less than satisfied.  Could be I didn't play enough with the parameters
  *         or didn't preprocess the data well enough... perhaps it could be usefully applied to
  * 		subregions defined using more common segmentation techinques.
  */

// ITK includes
#include <itkWatershedImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkRGBPixel.h>
#include <itkScalarToRGBPixelFunctor.h>
#include <itkUnaryFunctorImageFilter.h>

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/StatusBar/StatusBar.h>
#include <Application/Filters/ITKFilter.h>
#include <Application/Filters/Actions/ActionWatershedFilter.h>

#include <iostream>
#include <vector>
#include <sstream>


#ifndef NDEBUG
//TIMING STUFF
//		Courtesy of https://github.com/arhuaco/junkcode/blob/master/emqbit-bench/bench.c
#include <sys/time.h>
typedef unsigned long long timestamp_t;

static timestamp_t
get_timestamp ()
{
  struct timeval now;
  gettimeofday (&now, NULL);
  return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}
#endif

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
// NOTE: Registration needs to be done outside of any namespace
CORE_REGISTER_ACTION( Seg3D, WatershedFilter )




namespace Seg3D
{

typedef double WtrLabelType; //WtrLabelType
typedef WtrLabelType WtrPixelTypeOut; //WtrPixelTypeOut WtrLabelType // Should be the same
typedef itk::Image< WtrPixelTypeOut, 3 > WtrImageTypeOut; //WtrImageTypeOut itk::Image< WtrPixelTypeOut, 3 >
typedef itk::Image< unsigned long int, 3 > WtrImageTypeFilter; //WtrImageTypeFilter itk::Image< unsigned long int, 3 > 

bool ActionWatershedFilter::validate( Core::ActionContextHandle& context )
{
	// Make sure that the sandbox exists
	if ( !LayerManager::CheckSandboxExistence( this->sandbox_, context ) ) return false;

	// Check for layer existence and type information
	if ( ! LayerManager::CheckLayerExistenceAndType( this->target_layer_, 
												  Core::VolumeType::DATA_E, context, this->sandbox_ ) ) return false;

	// Check for layer availability 
	if ( ! LayerManager::CheckLayerAvailabilityForProcessing( this->target_layer_, context, this->sandbox_ ) ) return false;

	// make sure parameters are valid
	if( this->watershedThreshold_val_ < 0.0 || this->watershedThreshold_val_ > 1.0 )
	{
		context->report_error( "The threshold must be between 0 and 1." );
		return false;
	}
	if( this->watershedLevel_val_ < 0.0 || this->watershedLevel_val_ > 1.0 )
	{
		context->report_error( "The level must be between 0 and 1." );
		return false;
	}

	// Validation successful
	return true;
}


// ALGORITHM CLASS
// This class does the actual work and is run on a separate thread.
// NOTE: The separation of the algorithm into a private class is for the purpose of running the
// filter on a separate thread.
class WatershedFilterAlgo : public ITKFilter
{

public:
	LayerHandle src_layer_;
	//std::vector<LayerHandle> dst_layer_;
	LayerHandle dst_layer_;
	
	double watershedThreshold_val_;
	double watershedLevel_val_;

	std::vector<std::string> dst_layer_ids_;
	std::vector<WtrLabelType> watershed_image_output_labels_;

public:
	
	// RUN:
	// Implemtation of run of the Runnable base class, this function is called when the thread
	// is launched.

	// NOTE: The macro needs a data type to select which version to run. This needs to be
	// a member variable of the algorithm class.
	SCI_BEGIN_TYPED_ITK_RUN( this->src_layer_->get_data_type() )
	{
		CORE_LOG_DEBUG( this->get_filter_name() + ": Initializing watershedding filter..." );
		
		// Define the type of filter that we use.
		typedef itk::WatershedImageFilter< TYPED_IMAGE_TYPE > filter_type;
		
		// Retrieve the image as an itk image from the underlying data structure
		// NOTE: This only does wrapping and does not regenerate the data.
		typename Core::ITKImageDataT<VALUE_TYPE>::Handle input_image; 
		this->get_itk_image_from_layer<VALUE_TYPE>( this->src_layer_, input_image );
		
		// Create a new ITK filter instantiation.	
		typename filter_type::Pointer filter = filter_type::New();
		
		// Relay abort and progress information to the layer that is executing the filter.
		this->forward_abort_to_filter( filter, this->dst_layer_ );
		this->observe_itk_progress( filter, this->dst_layer_ );
		
		filter->SetLevel( this->watershedLevel_val_ );
		filter->SetThreshold( this->watershedThreshold_val_ );
		filter->SetInput( input_image->get_image() );
		
		// Ensure we will have some threads left for doing something else
		this->limit_number_of_itk_threads( filter );
		
		// Run the actual ITK filter.
		// This needs to be in a try/catch statement as certain filters throw exceptions when they
		// are aborted. In that case we will relay a message to the status bar for information.
		CORE_LOG_DEBUG( this->get_filter_name() + ": Updating watershedding filter..." );
		try{ 
			filter->Update(); 
		} 
		catch ( ... ){
			if ( this->check_abort() )
			{
				this->report_error( "Filter was aborted." );
				return;
			}
			this->report_error( "ITK filter failed to complete." );
			return;		
		}
		
		// As ITK filters generate an inconsistent abort behavior, we record our own abort flag
		// This one is set when the abort button is pressed and an abort is sent to ITK.
		if ( this->check_abort() ) return;
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Casting filter to output type..." );
		typedef itk::CastImageFilter< WtrImageTypeFilter, WtrImageTypeOut > CastImageFilter_FilterToOut;
		CastImageFilter_FilterToOut::Pointer castImage_FilterToOut = CastImageFilter_FilterToOut::New();
		castImage_FilterToOut->SetInput( filter->GetOutput() );
		castImage_FilterToOut->Update();
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Reading in all labels (extreme patience required)..." );
#ifndef NDEBUG
		timestamp_t t0 = get_timestamp();
#endif
		// Relabel the output
		// The watershedding algorithm splits everything up into labels.  So let's get em all and then output each label into it's own mask
		// This is from Dr. Jurrus's example ITK Watershed example.  No specific author is apparent.  I have a copy of the code if needed.
		itk::ImageRegionIterator< WtrImageTypeOut > wtrImg_iter ( castImage_FilterToOut->GetOutput(), filter->GetOutput()->GetLargestPossibleRegion() );
		int iterCount = 0;
		for( wtrImg_iter.GoToBegin(); !wtrImg_iter.IsAtEnd(); ++wtrImg_iter )
		{
		   iterCount += 1;
		   WtrLabelType val = wtrImg_iter.Get();
		   if ( find( this->watershed_image_output_labels_.begin(), this->watershed_image_output_labels_.end(), val ) == this->watershed_image_output_labels_.end() ){
			   this->watershed_image_output_labels_.push_back( val );
			}
		}
#ifndef NDEBUG
		timestamp_t t1 = get_timestamp();
		double secs = (t1 - t0) / 1000000.0L;
		
		std::ostringstream oss_msg;
		oss_msg << "\t" << iterCount << " iterations for " << this->watershed_image_output_labels_.size() << " labels in " << secs << " seconds.";
		CORE_LOG_DEBUG( oss_msg.str() );
#endif
		CORE_LOG_DEBUG( this->get_filter_name() + ": Creating new labels..." );
		
		std::vector< WtrLabelType > new_labels;
		// Could be an issue if the size of watershed_image_ouput_labels is larger than unsigned int can handle.  Doubt it, but still...
		for( unsigned int n=1; n<=this->watershed_image_output_labels_.size(); n++ ){
			new_labels.push_back( n );
		}
/* Better version for when RGB data types are available.  This code is untested.
 * 		Look into https://itk.org/Doxygen46/html/Segmentation_2WatershedSegmentation1_8cxx-example.html
 * 		to see a (probably) better way to convert the watershed output to an RGB image.  Specifically look
 * 		to the code following itk::Functor::ScalarToRGBPixelFunctor<unsigned long> ColorMapFunctorType;
 * Needs random library.  Random code from http://stackoverflow.com/a/19728404
 * 
 * --[CODE]--
 * #include <random>
 *		
 * typedef itk::RGBPixel< WtrLabelType > RGBPixelType;
 * typedef itk::Image< RGBPixelType, 3 > RGBImageType;
 * 
 * 	std::vector< RGBPixelType > watershed_image_ouput_rgb_labels; // As a member of WatershedFilterAlgo class
 * 
 * 		typedef itk::CastImageFilter< WtrImageTypeOut, RGBImageType > CastImageFilter_OutToRGB; // Would be more efficient to make RGB the Out
 *		CastImageFilter_OutToRGB::Pointer castImage_OutToRGB = CastImageFilter_OutToRGB::New();
 *		castImage_OutToRGB->SetInput( castImage_FilterToOut->GetOutput() );
 *		castImage_OutToRGB->Update();
 * 
 * 		std::random_device rd;     // only used once to initialise (seed) engine
 * 		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
 * 		std::uniform_int_distribution<int> uni(1,255); // guaranteed unbiased.  1 - 255 with 0 left for non segmented regions
 * 		RGBPixelType tmpRGB;
 *		for( std::size_t n = 1; n<=this->watershed_image_output_labels_.size(); n++ ){
 * 			// check code from http://www.cplusplus.com/forum/general/7784/#msg36171
 * 			bool check;
 * 			do{
 * 				tmpRGB[0] = uni(rng);
 * 				tmpRGB[1] = uni(rng);
 * 				tmpRGB[2] = uni(rng);
 * 				check=true;
 * 				for( int j=0; j<n; j++ ){
 * 					if( (tmpRGB[0] == this->watershed_image_ouput_rgb_labels[j][0]) && 
 * 						(tmpRGB[1] == this->watershed_image_ouput_rgb_labels[j][1]) && 
 * 						(tmpRGB[2] == this->watershed_image_ouput_rgb_labels[j][2]) ){ // if already used
 * 						check = false;
 * 						break;
 * 					}
 * 				}
 * 			}while( !check );
 * 			this->watershed_image_ouput_rgb_labels.push_back( tmpRGB );
 * 		}
 * --[END CODE]--
 * 
 * Note that random_shuffle and mapping code would no longer be needed.
 * The wtrImg_iter.Set code needs to be modified use to a RGB ImageRegionIterator
 * on the castImage_OutToRGB image object in order to use the rgb labels.  Also,
 * make sure to insert the rgb image into the layer
 */ 		
 
		CORE_LOG_DEBUG( this->get_filter_name() + ": Shuffling new labels..." );
		random_shuffle( new_labels.begin(), new_labels.end() );
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Mapping old labels to new..." );
		std::map< WtrLabelType, WtrLabelType > label_map;
		for( unsigned int i=0; i<this->watershed_image_output_labels_.size(); i++ ){
			label_map[ this->watershed_image_output_labels_[i] ] = new_labels[i];
		}
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Setting new labels..." );
		for( wtrImg_iter.GoToBegin(); !wtrImg_iter.IsAtEnd(); ++wtrImg_iter )
		{
			wtrImg_iter.Set( label_map[ wtrImg_iter.Get() ] );
		}
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Linking new labels..." );
		this->watershed_image_output_labels_ = new_labels;
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Inserting output into layer..." );
		this->insert_itk_image_into_layer( this->dst_layer_, castImage_FilterToOut->GetOutput() );
		
		CORE_LOG_DEBUG( this->get_filter_name() + ": Finished.\n" );
	}
	SCI_END_TYPED_ITK_RUN()
  
	// GET_FITLER_NAME:
	// The name of the filter, this information is used for generating new layer labels.
	virtual std::string get_filter_name() const
	{
		return "Watershed Filter";
	}

  // GET_LAYER_PREFIX:
  // This function returns the name of the filter. The latter is prepended to the new layer name,
  // when a new layer is generated.
  virtual std::string get_layer_prefix() const
  {
    return "Watershed";
  }
};


bool ActionWatershedFilter::run( Core::ActionContextHandle& context, 
                                       Core::ActionResultHandle& result )
{
	// Create algorithm
	boost::shared_ptr<WatershedFilterAlgo> algo( new WatershedFilterAlgo );

	// Copy the parameters over to the algorithm that runs the filter
	algo->set_sandbox( this->sandbox_ );
	algo->watershedThreshold_val_ = this->watershedThreshold_val_;
	algo->watershedLevel_val_ = this->watershedLevel_val_;

	// Find the handle to the layer
	if ( !( algo->find_layer( this->target_layer_, algo->src_layer_ ) ) )
	{
		return false;			
	}

	// Lock the src layer, so it cannot be used else where
	algo->lock_for_use( algo->src_layer_ );

	// Create the destination layer, which will show progress
	algo->create_and_lock_data_layer_from_layer( algo->src_layer_, algo->dst_layer_ );

	// Return the id of the destination layer.
	result = Core::ActionResultHandle( new Core::ActionResult( algo->dst_layer_->get_layer_id() ) );
	// If the action is run from a script (provenance is a special case of script),
	// return a notifier that the script engine can wait on.
	if ( context->source() == Core::ActionSource::SCRIPT_E ||
	  context->source() == Core::ActionSource::PROVENANCE_E )
	{
		context->report_need_resource( algo->get_notifier() );
	}

	// Build the undo-redo record
	algo->create_undo_redo_and_provenance_record( context, this->shared_from_this() );

	// Start the filter.
	Core::Runnable::Start( algo );

	return true;
}


void ActionWatershedFilter::Dispatch( Core::ActionContextHandle context, std::string target_layer,
                                      double watershedThreshold_val, double watershedLevel_val )
{	
	// Create a new action
	ActionWatershedFilter* action = new ActionWatershedFilter;

	// Setup the parameters
	action->target_layer_ = target_layer;
	action->watershedThreshold_val_ = watershedThreshold_val;
	action->watershedLevel_val_ = watershedLevel_val;

	// Dispatch action to underlying engine
	Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D
