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

// Boost includes
#include <boost/thread.hpp>

// Core includes
#include <Core/Application/Application.h>

// Application includes
#include <Application/Provenance/Provenance.h>



namespace Seg3D
{

class ProvenanceCounter 
{
public:
  ProvenanceCounter() :
    count_ ( 0 )
  {
  }

  ProvenanceID generate()
  {
    lock_type lock( this->mutex_ );
    count_++;
    return count_;
  }

  ProvenanceID get()
  {
    lock_type lock( this->mutex_ );
    return count_;
  }

  void set( ProvenanceID count )
  {
    lock_type lock( this->mutex_ );
    count_ = count;
  }

private:
  typedef boost::mutex::scoped_lock lock_type;
  boost::mutex mutex_;
  ProvenanceID count_;
};

static ProvenanceCounter ProvenanceCounter;

ProvenanceID GenerateProvenanceID()
{
  return ProvenanceCounter.generate();
}

ProvenanceID GetProvenanceCount()
{
  return ProvenanceCounter.get(); 
}

void SetProvenanceCount( ProvenanceID count )
{
  ProvenanceCounter.set( count );
}

} // end namespace Seg3D
