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

// File         : AsyncMosaicSave.h
// Author       : Bradley C. Grimm
// Created      : 2009/06/25
// Copyright    : (C) 2009 University of Utah
// Description  : An asyncronous save manager so that ir-assemble
//                progress is not halted with each save.

#ifndef __ASYNC_MOSAIC_SAVE_H__
#define __ASYNC_MOSAIC_SAVE_H__

#include <Core/ITKCommon/ThreadUtils/the_boost_thread.hxx>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

//----------------------------------------------------------------
// save_mosaic_t
// 
template <typename T>
class save_mosaic_t : public the_transaction_t
{
public:
  save_mosaic_t(const T * image, 
                const bfs::path & filename,
                bool blab = true):
  m_image(image),
  m_filename(filename),
  m_blab(blab)
  {
  }
  
  // virtual:
  void execute(the_thread_interface_t * thread)
  {
    typedef typename itk::ImageFileWriter<T> writer_t;
    typename writer_t::Pointer writer = writer_t::New();
    writer->SetInput(m_image);
    
    if (m_blab) std::cout << "saving " << m_filename << std::endl;
    writer->SetFileName(m_filename.string().c_str());
    writer->Update();
  }
  
  // image:
  typename T::ConstPointer m_image;
  
  // filename:
  bfs::path m_filename;
  
  bool m_blab;
};


template <typename T> 
void save_mosaic_async(const T * image, 
                       const bfs::path & filename, 
                       the_thread_pool_t &thread_pool,
                       bool blab = true)
{
  std::list<the_transaction_t *> schedule;
  save_mosaic_t<T> * t = new save_mosaic_t<T>(image, filename, blab);
  schedule.push_back(t);
  
  // setup the thread pool:
  thread_pool.push_back(schedule);
  //thread_pool.pre_distribute_work();
  thread_pool.start();
}

#endif


