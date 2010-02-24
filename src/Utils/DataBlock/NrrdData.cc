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

#include <Utils/Math/MathFunctions.h>
#include <Utils/DataBlock/NrrdData.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

namespace Utils {

NrrdData::NrrdData(Nrrd* nrrd, bool own_data) :
  nrrd_(nrrd),
  own_data_(own_data)
{
}   
    
NrrdData::~NrrdData()
{
  // If we own the data, clear the nrrd
  if (own_data_) 
  {
    nrrdNuke(nrrd_);
  }
  else
  {
    nrrdNix(nrrd_);
  }
}

Transform 
NrrdData::transform() const
{
  Transform transform;
  transform.load_identity();

  if (nrrd_ == 0) return transform;
  
  size_t dim = nrrd_->dim;

  std::vector<double> min(dim,0.0), max(dim,0.0);
  std::vector<size_t> size(dim,0);
  
  // Extract info of oldest versions of nrrds
  for (size_t p=0; p<dim; p++) 
  {
    size[p] = nrrd_->axis[p].size;
    
    if (IsFinite(nrrd_->axis[p].min)) 
    {
      min[p] = nrrd_->axis[p].min;    
    }
    else
    {
      min[p] = 0.0;
    }
    
    if (IsFinite(nrrd_->axis[p].max)) 
    {
      max[p] = nrrd_->axis[p].max;
    }
    else
    {
      if (IsFinite(nrrd_->axis[p].spacing)) 
      {
        max[p] = nrrd_->axis[p].spacing*size[p];        
      }
      else
      {
        max[p] = static_cast<double>(size[p]-1);        
      }
    }
  }  

  size_t k = 0;
  for (size_t p=0; p< dim; p++)   
  {
    if (size[p] == 1) continue;
    k++;
  }
  size_t rdim = k;

  std::vector<double> rmin(rdim), rmax(rdim);
  std::vector<int> rsize(rdim);
  
  k = 0;
  for (size_t p=0; p< dim; p++)   
  {
    if (size[p] == 1) continue;
    rmin[k] = min[p];
    rmax[k] = max[p];
    rsize[k] = size[p];
    k++;
  }

  if (rdim == 1)
  {
    Vector v0, v1, v2;
    v0 = Point(rmax[0],0.0,0.0) - Point(rmin[0],0.0,0.0);
    v0.find_orthogonal(v1,v2);

    transform.load_basis(Point(rmin[0],0.0,0.0),v0,v1,v2);
    transform.post_scale(Vector(1.0/static_cast<double>(rsize[0]), 
                         1.0, 1.0));    
  }
  else if (rdim == 2)
  {
    Vector v0, v1, v2;
    v0 = Point(rmax[0],0.0,0.0) - Point(rmin[0],0.0,0.0);
    v1 = Point(0.0,rmax[0],0.0) - Point(0.0,rmin[0],0.0);
    v2 = Cross(v0,v1);
    v2.normalize();

    transform.load_basis(Point(rmin[0],rmin[1],0.0),v0,v1,v2);
    transform.post_scale(Vector(1.0/static_cast<double>(rsize[0]), 
                                1.0/static_cast<double>(rsize[1]), 1.0));    
  }
  else if (rdim > 2)
  {
    Vector v0, v1, v2;
    v0 = Point(rmax[0],0.0,0.0) - Point(rmin[0],0.0,0.0);
    v1 = Point(0.0,rmax[0],0.0) - Point(0.0,rmin[0],0.0);
    v2 = Point(0.0,0.0,rmax[0]) - Point(0.0,0.0,rmin[0]);

    transform.load_basis(Point(rmin[0],rmin[1],rmin[2]),v0,v1,v2);
    transform.post_scale(Vector(1.0/static_cast<double>(rsize[0]), 
                                1.0/static_cast<double>(rsize[1]),
                                1.0/static_cast<double>(rsize[2])));    
  }

  if (nrrd_->spaceDim > 0)
  {
    Vector Origin;
    std::vector<Vector> SpaceDir(3);

    for (size_t p=0; p< rdim; p++)   
    {
      rmin[p] = 0.0;
      rmax[p] = static_cast<double>(rsize[p]);
      if (nrrd_->axis[p].center == 1)
      {
        double cor = (rmax[p]-rmin[p])/(2*rsize[p]);
        rmin[p] -= cor;
        rmax[p] -= cor;
      }
    }     
    
    if (nrrd_->spaceDim > 0)
    {
      if (IsFinite(nrrd_->spaceOrigin[0])) 
      {
        Origin.x(nrrd_->spaceOrigin[0]); 
      }
      
      for (size_t p=0; p<rdim && p < 3;p++)
      {
        if (IsFinite(nrrd_->axis[p].spaceDirection[0])) 
        {
          SpaceDir[p].x(nrrd_->axis[p].spaceDirection[0]);
        }
      }
    }

    if (nrrd_->spaceDim > 1)
    {
      if (IsFinite(nrrd_->spaceOrigin[1])) 
      {
        Origin.y(nrrd_->spaceOrigin[1]);
      }
      for (size_t p=0; p<rdim && p < 3;p++)
      {
        if (IsFinite(nrrd_->axis[p].spaceDirection[1])) 
        {
          SpaceDir[p].y(nrrd_->axis[p].spaceDirection[1]);
        }
      }
    }
    
    if (nrrd_->spaceDim > 2)
    {
      if (IsFinite(nrrd_->spaceOrigin[2]))
      {
        Origin.z(nrrd_->spaceOrigin[2]);
      }
      for (size_t p=0; p<rdim && p < 3;p++)
      {
        if (IsFinite(nrrd_->axis[p].spaceDirection[2])) 
        {
          SpaceDir[p].z(nrrd_->axis[p].spaceDirection[2]);
        }
      }
    }
  
    if (dim == 1) 
    {
      SpaceDir[0].find_orthogonal(SpaceDir[1],SpaceDir[2]);
    }
    else if (dim == 2)
    {
      SpaceDir[2] = Cross(SpaceDir[0],SpaceDir[1]);
    }
 
    Transform space_transform;
    space_transform.load_basis(Point(Origin),SpaceDir[0],SpaceDir[1],SpaceDir[2]);
    transform.pre_transform(space_transform);
  }

  return transform;
}


void
NrrdData::set_transform(Transform& transform)
{
  if (!nrrd_) return;
  
  int centerdata[NRRD_DIM_MAX];
  for (int p=0;p<NRRD_DIM_MAX;p++)
  {
    centerdata[p] = nrrdCenterCell;
  }

  nrrdAxisInfoSet_nva(nrrd_,nrrdAxisInfoCenter,centerdata); 

  int kind[NRRD_DIM_MAX];
  for (int p=0;p<NRRD_DIM_MAX;p++)
  {
    kind[p] = nrrdKindSpace;
  }
  nrrdAxisInfoSet_nva(nrrd_,nrrdAxisInfoKind,kind);

  nrrd_->spaceDim = 3;
  
  double Trans[16];

  transform.get(Trans);
  for (int p=0;p<3;p++)
  {
    nrrd_->spaceOrigin[p] = Trans[3+4*p];
    for (size_t q=0;q<nrrd_->dim;q++)
      nrrd_->axis[q].spaceDirection[p] = Trans[q+4*p];
  }
  
  for (int p=0;p<3;p++)
    for (int q=0;q<3;q++)
    {
      if (p==q) nrrd_->measurementFrame[p][q] = 1.0;
      else nrrd_->measurementFrame[p][q] = 0.0;
    }
  
  nrrd_->space = nrrdSpace3DRightHanded;

}  

  
size_t 
NrrdData::nx() const
{
  if (nrrd_ && nrrd_->dim > 0) return nrrd_->axis[0].size;
  return 1;
}


size_t 
NrrdData::ny() const
{
  if (nrrd_ && nrrd_->dim > 1) return nrrd_->axis[1].size;
  return 1;
}


size_t 
NrrdData::nz() const
{
  if (nrrd_ && nrrd_->dim > 2) return nrrd_->axis[2].size;
  return 1;
}


bool 
NrrdData::is_char() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeChar) return true;
  return false;
}


bool 
NrrdData::is_unsigned_char() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeUChar) return true;
  return false;
}


bool 
NrrdData::is_short() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeShort) return true;
  return false;
}


bool 
NrrdData::is_unsigned_short() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeUShort) return true;
  return false;
}


bool 
NrrdData::is_int() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeInt) return true; 
  return false;
}


bool 
NrrdData::is_unsigned_int() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeUInt) return true; 
  return false;
}


bool 
NrrdData::is_longlong() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeLLong) return true; 
  return false;
}


bool 
NrrdData::is_unsigned_longlong() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeULLong) return true; 
  return false;
}


bool 
NrrdData::is_float() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeFloat) return true; 
  return false;
}


bool 
NrrdData::is_double() const
{
  if (nrrd_ && nrrd_->type == nrrdTypeDouble) return true; 
  return false;
}



bool
NrrdData::LoadNrrd(const std::string& filename, 
                    NrrdDataHandle& nrrddata, 
                    std::string& error)
{
  // Lock down the Teem library
  lock_type lock(GetMutex());

  Nrrd* nrrd = nrrdNew();
  if (nrrdLoad(nrrd, filename.c_str(), 0)) 
  {
    char *err = biffGet(NRRD);
    error = std::string("Could not open file: ") + filename + " : " +
            std::string(err);
    free(err);
    biffDone(NRRD);
    
    nrrddata.reset();
    return false;
  }  
  
  error = "";
  nrrddata = NrrdDataHandle(new NrrdData(nrrd));
  return true;
}


bool
NrrdData::SaveNrrd(const std::string& filename, 
                    NrrdDataHandle nrrddata, 
                    std::string& error)
{
  // Lock down the Teem library
  lock_type lock(GetMutex());
  // TODO:
  // Fix filename handling so it always ends in .nrrd or .nhdr

  if ( !(nrrddata.get()) )
  {
    error = "Error writing file: " + filename + " : no data volume available";
    return false;
  }

  if (nrrdSave(filename.c_str(), nrrddata->nrrd(), 0))
  {
    char *err = biffGet(NRRD);      
    error = "Error writing file: " + filename + " : " + std::string(err);
    free(err);
    biffDone(NRRD);

    return false;
  }

  error = "";
  return true;
}

// Need to store the static mutex somewhere
NrrdData::mutex_type NrrdData::teem_mutex_;

} // end namespace Utils
