/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

#include <Application/ImageRegistrationTools/Actions/ActionAddTransformsFilter.h>

// local includes:
#include <Core/ITKCommon/pyramid.hxx>
#include <Core/ITKCommon/match.hxx>
#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/grid_common.hxx>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/LineMath.h>
#include <Core/ITKCommon/STOS/stos.hxx>
#include <Core/ITKCommon/STOS/stos_common.hxx>
#include <Core/ITKCommon/Transform/itkGridTransform.h>
#include <Core/ITKCommon/Transform/itkMeshTransform.h>
#include <Core/ITKCommon/ThreadUtils/the_boost_mutex.hxx>
#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// ITK includes:
#include <itkCommand.h>

// system includes:
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>

#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

//----------------------------------------------------------------
// grid_transform_t
//
typedef itk::GridTransform grid_transform_t;

typedef std::list<bfs::path> pathList;


CORE_REGISTER_ACTION( Seg3D, AddTransformsFilter )

namespace Seg3D
{

using namespace ir;

//----------------------------------------------------------------
// GetAllEdges
//        Return all edges containing a certain point in a given
//        grid transform.
static std::vector<EdgeSegment> GetEdges(const the_grid_transform_t &transform,
                                         unsigned int pt_index)
{
  std::vector<EdgeSegment> edges;
  
  std::size_t num_tris = transform.grid_.tri_.size();
  for (unsigned int tri_idx = 0; tri_idx < num_tris; tri_idx++)
  {
    triangle_t warp_triangle = transform.grid_.tri_[tri_idx];
    
    for (unsigned int vertex_id = 0; vertex_id < 3; vertex_id++)
    {
      if ( pt_index == warp_triangle.vertex_[vertex_id] )
      {
        unsigned int tri_edge1 = (vertex_id + 1) % 3,
        tri_edge2 = (vertex_id + 2) % 3;
        
        unsigned int mesh_idx1 = warp_triangle.vertex_[tri_edge1],
        mesh_idx2 = warp_triangle.vertex_[tri_edge2];
        
        EdgeSegment line1(transform.grid_.mesh_[pt_index],
                          transform.grid_.mesh_[mesh_idx1]);
        EdgeSegment line2(transform.grid_.mesh_[pt_index],
                          transform.grid_.mesh_[mesh_idx2]);
        
        edges.push_back(line1);
        edges.push_back(line2);
      }
    }
  }
  
  return edges;
}

//----------------------------------------------------------------
// GetAllEdges
//        Return all edges in a given grid transform.
static std::vector<EdgeSegment> GetAllEdges(const the_grid_transform_t &transform)
{
  std::vector<EdgeSegment> edges;
  
  std::size_t num_tris = transform.grid_.tri_.size();
  for (unsigned int ft = 0; ft < num_tris; ft++)
  {
    for (unsigned int ft_idx = 0; ft_idx < 3; ft_idx++)
    {
      unsigned int fixed_pt1_idx = transform.grid_.tri_[ft].vertex_[ft_idx % 3],
      fixed_pt2_idx = transform.grid_.tri_[ft].vertex_[(ft_idx + 1) % 3];
      
      EdgeSegment line(transform.grid_.mesh_[fixed_pt1_idx],
                       transform.grid_.mesh_[fixed_pt2_idx]);
      edges.push_back(line);
    }
  }
  return edges;
}

bool
ActionAddTransformsFilter::validate( Core::ActionContextHandle& context )
{
  return true;
}

//----------------------------------------------------------------
// main
//
bool
ActionAddTransformsFilter::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  try
  {
    // this is so that the printouts look better:
    std::cout.precision(6);
    std::cout.setf(std::ios::scientific);
    
    // setup thread storage for the main thread:
    set_the_thread_storage_provider(the_boost_thread_t::thread_storage);
    the_boost_thread_t MAIN_THREAD_DUMMY;
    MAIN_THREAD_DUMMY.set_stopped(false);
    
    // setup thread and mutex interface creators:
    the_mutex_interface_t::set_creator(the_boost_mutex_t::create);
    the_thread_interface_t::set_creator(the_boost_thread_t::create);
    
    // parameters:
    pathList stos_fn;
    std::list<stos_t<image_t> > stos_list;
    pathList slice_dirs;
    
    pathList save_list;
    bfs::path fn_save;

    for (size_t i = 0; i < this->input_files_.size(); ++i)
    {
      bfs::path stosInput(this->input_files_[i]);
      // needed, since mosaic loading functions do not check for file existence
      if (! bfs::exists(stosInput) )
      {
        std::ostringstream oss;
        oss << "STOS input file " << stosInput << " not found. Skipping...";
        CORE_LOG_WARNING(oss.str());
      }
      else
      {
        stos_fn.push_back(stosInput);
      }
    }
    
    for (size_t i = 0; i < this->slice_dirs_.size(); ++i)
    {
      bfs::path sliceInput(this->slice_dirs_[i]);
      if (! bfs::exists(sliceInput) )
      {
        std::ostringstream oss;
        oss << "Slice input file " << sliceInput << " not found. Skipping...";
        CORE_LOG_WARNING(oss.str());
      }
      else
      {
        slice_dirs.push_back(sliceInput);
      }
    }
    
    if ( (slice_dirs.size() == 0) || ( (slice_dirs.size() > 1) && (slice_dirs.size() != stos_fn.size() + 1) ) )
    {
      context->report_error("There must be exactly one slice_dir for each mosaic listed in the input stos files, or one directory containing all slices.");
      return false;
    }

    for (size_t i = 0; i < this->output_prefixes_.size(); ++i)
    {
      // assuming prefix looks something like: my_output_dir/filestem,
      // so that output will be my_output_dir/filestem_*.*
      bfs::path save_prefix(this->output_prefixes_[i]);
      if (! bfs::is_directory( save_prefix.parent_path() ) )
      {
        CORE_LOG_DEBUG(std::string("Creating parent path to ") + this->output_prefixes_[i]);
        if (! boost::filesystem::create_directories( save_prefix.parent_path() ) )
        {
          std::ostringstream oss;
          oss << "Could not create missing directory " << save_prefix.parent_path() << " required to create output image.";
          context->report_error(oss.str());
          return false;
        }
      }
      save_list.push_back(save_prefix);
    }
    
    bool override_slice_dirs = ( slice_dirs.size() > 1 );
    
    // a minimum of two images are required for the registration:
    const unsigned int num_pairs = stos_fn.size();
    if (num_pairs == 0)
    {
      context->report_error("A minimum of two images are required for registration.");
      return false;
    }
    
    const unsigned int num_saves = save_list.size();
    if (num_saves == 0)
    {
      context->report_error("At least one output prefix is required.");
      return false;
    }
    
    // Load up all of the stos files.
    pathList::const_iterator slice_iter = slice_dirs.begin();
    for (pathList::const_iterator fn_iter = stos_fn.begin(); fn_iter != stos_fn.end(); ++fn_iter)
    {
      bfs::path slice0_dir, slice1_dir;
      
      if ( override_slice_dirs )
      {
        slice0_dir = *slice_iter;
        slice_iter++;
        slice1_dir = *slice_iter;
      }
      else
      {
        // use same slice directory throughout
        slice0_dir = *slice_iter;
        slice1_dir = *slice_iter;
      }
      
      stos_t<image_t> stos( *fn_iter, slice0_dir, slice1_dir );
      stos_list.push_back( stos );
    }
    
    bool use_as_prefix = false;
    if (num_saves != num_pairs)
    {
      if ( (num_saves == 1) && (save_list.front().extension().string() != ".stos") )
      {
        use_as_prefix = true;
        fn_save = save_list.front();
      }
      else
      {
        std::ostringstream oss;
        oss << "Must specify either a single prefix, or a new filename for each .stos input except the first one.";
        context->report_error(oss.str());
        return false;
      }
    }
    
    stos_tree_t<image_t> stos_tree;
    std::size_t num_roots = stos_tree.build(stos_list);
    
    if (num_roots != 1)
    {
      // TODO: log!
      if (num_roots == 0)
      {
        std::cerr << "ERROR: There is more than one possible mapping for "
        << "one or more slices, aborting... "
        << "Remove one of the ambiguous stos transforms and try again."
        << std::endl;
      }
      else
      {
        std::cerr << "ERROR: There are " << num_roots
        << " sets of interconnected stos transforms present, " << std::endl
        << " this tool only works with one set, aborting..." << std::endl;
      }
      stos_tree.dump(std::cerr);
      
      return false;
    }
    
    // this will sort the transforms in order of dependence and alphabetically:
    stos_list.clear();
    stos_tree.collect_stos(stos_list);
    
    // setup random access vectors:
    std::vector<stos_t<image_t> > stos(stos_list.begin(), stos_list.end());
    std::vector<bfs::path> saves_names(save_list.begin(), save_list.end());
    std::vector<itk::GridTransform::Pointer> grid_transforms(num_pairs);
    std::vector<itk::MeshTransform::Pointer> mesh_transforms(num_pairs);
    
    for (unsigned int i = 0; i < num_pairs; i++)
    {
      base_transform_t::Pointer transform_stos =
      const_cast<base_transform_t *>(stos[i].t01_.GetPointer());
      
      grid_transforms[i] =
      dynamic_cast<itk::GridTransform *>(transform_stos.GetPointer());
      
      if ( grid_transforms[i].GetPointer() == NULL )
      {
        std::cerr << stos[i].fn_load_ << " is not a grid transform.  Currently "
        << "ir-add-transforms only supports grid transforms." << std::endl;
        return false;
      }
      
      // Setup the new mesh transform.
      mesh_transforms[i] = itk::MeshTransform::New();
      mesh_transforms[i]->transform_.tile_min_ =
      grid_transforms[i]->transform_.tile_min_;
      mesh_transforms[i]->transform_.tile_ext_ =
      grid_transforms[i]->transform_.tile_ext_;
      mesh_transforms[i]->transform_.grid_.resize(
        grid_transforms[i]->transform_.grid_.rows_,
        grid_transforms[i]->transform_.grid_.cols_);
      
      // The first stos has no need to be added to...
      if ( i == 0 )
      {
        // Transform the points exactly.
        std::size_t num_pts = grid_transforms[0]->transform_.grid_.mesh_.size();
        for (unsigned int k = 0; k < num_pts; k++)
        {
          bool delay_setup = (i != num_pairs - 1);
          mesh_transforms[0]->transform_.insert_point(
            grid_transforms[0]->transform_.grid_.mesh_[k].uv_,
            grid_transforms[0]->transform_.grid_.mesh_[k].xy_,
            delay_setup);
        }
      }
    }
    
    // Create new 'added' transforms for each stos except the first.
    for (unsigned int i = 0; i < num_pairs; i++)
    {
      // Transform each of the grid points.
      std::size_t num_pts = grid_transforms[i]->transform_.grid_.mesh_.size();
      // Just save out the first slice.  No addition necessary.
      if ( i == 0 )
        continue;
      
      the_grid_transform_t fixed_transform = grid_transforms[i-1]->transform_;
      the_grid_transform_t warp_transform = grid_transforms[i]->transform_;
      for (unsigned int k = 0; k < num_pts; k++)
      {
        // Check if point can be transformed...
        pnt2d_t xy_transformed;
        pnt2d_t uv_transformed =
        PointMath::XYtoUV(warp_transform.grid_.mesh_[k].xy_,
          fixed_transform.tile_min_,
          fixed_transform.tile_ext_);
        
        bool success = fixed_transform.transform_inv(uv_transformed,
                                                     xy_transformed);
        
        if ( success )
        {
          // Transformed successfully
          mesh_transforms[i]->transform_.insert_point(
                                                      warp_transform.grid_.mesh_[k].uv_,
                                                      xy_transformed,
                                                      true);
        }
        else
        {
          // Could not be transformed.
          
          // Grab edges that include point, as well as all fixed transform edges.
          std::vector<EdgeSegment> fixed_edges = GetAllEdges(fixed_transform);
          unsigned int num_fixed_edges = fixed_edges.size();
          
          std::vector<EdgeSegment> edges = GetEdges(warp_transform, k);
          unsigned int num_edges = edges.size();
          for (unsigned int edge_idx = 0; edge_idx < num_edges; edge_idx++)
          {
            // Convert to XY space for segment checks.
            edges[edge_idx].ToXY(warp_transform.tile_min_,
                                 warp_transform.tile_ext_);
            
            EdgeSegment found_edge;
            pnt2d_t intersect, found_intersect;
            double nearest_intersect = std::numeric_limits<double>::max();
            
            // Find the closest edge intersecting the point.
            for (unsigned int fix_idx = 0; fix_idx < num_fixed_edges; fix_idx++)
            {
              fixed_edges[fix_idx].ToXY(fixed_transform.tile_min_,
                                        fixed_transform.tile_ext_);
              double distance =
              edges[edge_idx].SegmentsIntersect(fixed_edges[fix_idx], intersect);
              
              if( distance < nearest_intersect )
              {
                found_edge = fixed_edges[fix_idx];
                nearest_intersect = distance;
                found_intersect = intersect;
              }
            }
            
            if ( nearest_intersect == std::numeric_limits<double>::max() )
              continue;
            
            // Is this point inside the grid?
            pnt2d_t xy_transformed;
            pnt2d_t uv_transformed =
            PointMath::XYtoUV(edges[edge_idx].pt2.xy_,
                              warp_transform.tile_min_,
                              warp_transform.tile_ext_);
            bool success = fixed_transform.transform_inv(uv_transformed,
                                                         xy_transformed);
            if ( !success )
              continue;
            
            // Determine our new ctrl (xy) point.
            double dist_to_point =
            PointMath::Distance(found_edge.pt1.uv_, found_intersect);
            double found_uv_length =
            PointMath::Distance(found_edge.pt1.uv_, found_edge.pt2.uv_);
            
            double map_fraction = dist_to_point / found_uv_length;
            pnt2d_t ctrl_point = PointMath::Scale(found_edge.pt1.xy_,
                                                  found_edge.pt2.xy_,
                                                  map_fraction);
            
            // Determine our new map (uv) point.
            double xy_length = PointMath::Distance(edges[edge_idx].pt1.xy_,
                                                   edges[edge_idx].pt2.xy_);
            double fraction = nearest_intersect / xy_length;
            fraction = (fraction > 1) ? 1 : ((fraction < 0) ? 0 : fraction);
            pnt2d_t map_point = PointMath::Scale(edges[edge_idx].pt1.uv_,
                                                 edges[edge_idx].pt2.uv_,
                                                 fraction);
            
            // Convert point back to uv space.
            map_point = PointMath::XYtoUV(map_point, 
                                          warp_transform.tile_min_, 
                                          warp_transform.tile_ext_);
            
            // Add to new mesh transform.
            mesh_transforms[i]->transform_.insert_point(map_point, 
                                                        ctrl_point, 
                                                        true);
          }
        }
      }
      
      // Save out new transform.
      stos[i].t01_ = mesh_transforms[i];
      if ( use_as_prefix )
      {
//        the_text_t fn_filename = fn_save + the_text_t::number(i, 3, '0');
//        the_text_t fn_extension(".stos");
//        the_text_t fn_numbered_save = fn_filename + fn_extension;
        std::ostringstream fn_filename;
        fn_filename << fn_save.string() << "_" << std::setfill('0') << std::setw(3) << i << ".stos";
        bfs::path fn_numbered_save(fn_filename.str());
        stos[i].save(fn_numbered_save);
      }
      else
      {
        stos[i].save(saves_names[i]);
      }
    }

    CORE_LOG_SUCCESS("ir-add-transforms done");

    return true;
  }
  catch (bfs::filesystem_error &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (itk::ExceptionObject &err)
  {
    CORE_LOG_ERROR(err.GetDescription());
  }
  catch (Core::Exception &err)
  {
    CORE_LOG_ERROR(err.what());
    CORE_LOG_ERROR(err.message());
  }
  catch (std::exception &err)
  {
    CORE_LOG_ERROR(err.what());
  }
  catch (...)
  {
    CORE_LOG_ERROR("Unknown exception type caught.");
  }
  return false;
}

void
ActionAddTransformsFilter::Dispatch(Core::ActionContextHandle context,
                                    std::string target_layer,
                                    std::vector<std::string> input_files,
                                    std::vector<std::string> output_prefixes,
                                    std::vector<std::string> slice_dirs)
{
  // Create a new action
  ActionAddTransformsFilter* action = new ActionAddTransformsFilter;
  
  // Setup the parameters
  action->target_layer_ = target_layer;
  action->input_files_ = input_files;
  action->output_prefixes_ = output_prefixes;
  action->slice_dirs_ = slice_dirs;
  
  // Dispatch action to underlying engine
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

}