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

// File         : mosaic_layout_common.cxx
// Author       : Pavel A. Koshevoy
// Created      : Thu Mar 22 13:10:41 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// Description  : Helper functions for automatic mosaic layout.

// the includes:
#include <Application/ImageRegistrationTools/Utils/MosaicLayout.h>


// ITK includes:
#include <itkCommand.h>
#include <itkImageMaskSpatialObject.h>
#include <itkImageRegistrationMethod.h>
#include <itkMeanSquaresImageToImageMetric.h>


//----------------------------------------------------------------
// reset
// 
void
reset(const unsigned int num_images,
      const unsigned int max_cascade_len,
      array3d(translate_transform_t::Pointer) & path,
      array3d(double) & cost)
{
  for (unsigned int cascade = 0; cascade <= max_cascade_len; cascade++)
  {
    for (unsigned int i = 0; i < num_images; i++)
    {
      for (unsigned int j = 0; j < num_images; j++)
      {
        path[cascade][i][j] = 0;
        cost[cascade][i][j] = std::numeric_limits<double>::max();
      }
    }
  }
}

//----------------------------------------------------------------
// assemble_cascades
// 
void
assemble_cascades(const unsigned int num_images,
                  const unsigned int max_cascade_len,
                  array3d(translate_transform_t::Pointer) & path,
                  array3d(double) & cost,
                  const bool & cumulative_cost)
{
  for (unsigned int intermediate = 1;
       intermediate <= max_cascade_len;
       intermediate++)
  {
    for (unsigned int i = 0; i < num_images; i++)
    {
      for (unsigned int j = i + 1; j < num_images; j++)
      {
        // attempt to establish a mapping between images i and j
        // via an intermediate mapping:
        
        translate_transform_t::Pointer best_ij = 0;
        double best_metric = std::numeric_limits<double>::max();
        
        for (unsigned int sa = 0; sa < intermediate; sa++)
        {
          unsigned int sb = intermediate - sa - 1;
          
          for (unsigned int k = 0; k < num_images; k++)
          {
            if (k == j) continue;
            
            double metric_ab = std::max(cost[sa][i][k], cost[sb][k][j]);
            if (cumulative_cost)
            {
              metric_ab = cost[sa][i][k] + cost[sb][k][j];
            }
            
            if (metric_ab >= best_metric) continue;
            
            // cascade the translation:
            best_ij = translate_transform_t::New();
            best_ij->SetOffset(path[sa][i][k]->GetOffset() +
                               path[sb][k][j]->GetOffset());
            best_metric = metric_ab;
          }
        }
        
        if (best_metric == std::numeric_limits<double>::max()) continue;
        
        path[intermediate][i][j] = best_ij;
        path[intermediate][j][i] = inverse(best_ij);
        cost[intermediate][i][j] = best_metric;
        cost[intermediate][j][i] = best_metric;
      }
    }
  }
}

//----------------------------------------------------------------
// establish_mappings
// 
void
establish_mappings(/*the_log_t & log,*/
                   const unsigned int num_images,
                   const unsigned int max_cascade_len,
                   const array3d(translate_transform_t::Pointer) & path,
                   const array3d(double) & cost,
                   array2d(translate_transform_t::Pointer) & mapping,
                   array2d(double) & mapping_cost)
{
  for (unsigned int i = 0; i < num_images; i++)
  {
    // there is always a mapping from an image to itself:
    mapping[i][i] = translate_transform_t::New();
    mapping_cost[i][i] = 0.0;
    
    for (unsigned int j = i + 1; j < num_images; j++)
    {
      mapping[i][j] = 0;
      mapping[j][i] = 0;
      mapping_cost[i][j] = std::numeric_limits<double>::max();
      mapping_cost[j][i] = std::numeric_limits<double>::max();
      
      // find the shortest transformation chain between images i, j:
      double best_metric = std::numeric_limits<double>::max();
      for (unsigned int intermediate = 0;
           intermediate <= max_cascade_len;
           intermediate++)
      {
        if (cost[intermediate][i][j] >= best_metric) continue;
        
        best_metric = cost[intermediate][i][j];
        
        mapping[i][j] = path[intermediate][i][j];
        mapping[j][i] = path[intermediate][j][i];
        mapping_cost[i][j] = best_metric;
        mapping_cost[j][i] = best_metric;
        
//        log << "best mapping: " << j << "->" << i << " has "
//        << intermediate << " intermediate steps, metric: "
//        << best_metric << endl;
      }
    }
  }

//  log << endl;
}