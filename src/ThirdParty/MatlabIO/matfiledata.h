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


// NOTE: This MatlabIO file is used in different projects as well. Please, do not
// make it depend on other scirun code. This way it is easier to maintain matlabIO 
// code among different projects. Thank you.

/*
 * FILE: matfile.h
 * AUTH: Jeroen G Stinstra
 * DATE: 16 MAY 2005
 */
 
#ifndef JGS_MATLABIO_MATFILEDATA_H
#define JGS_MATLABIO_MATFILEDATA_H 1

/*
 * The matfiledata class is a small support class for the matfile class
 * it contains the data fragments read from disk. In fact the class is 
 * a handle to the data segment. Hence Copying the object will only copy
 * the handle.
 * 
 * The class also helps converting the data between various formats 
 *
 */


/*
 * CLASS DESCRIPTION
 * This class is a container for storing large quantities of numeric data. In this
 * case the data of an array. In order to ease the memory management, this class
 * acts as a handle and data object in one. Copying the object only copies the handle
 * on the other hand destroying all handles will result in freeing the memory.
 *
 * MEMORY MODEL
 * The class maintains its own copies of the data. Each vector, string and other
 * data unit is copied. Data being imported into the container is being copied and
 * casted. The object obtains its own memory and frees this as well. The functions
 * for importing and exporting data allow you to specify a pointer where data can be
 * written or is stored, the function does not free or allocate any of memory blocks.
 *
 * ERROR HANDLING
 * All errors are reported as exceptions described in the matfilebase class.
 * Errors of external c library functions are caught and forwarded as exceptions.
 *
 * COPYING/ASSIGNMENT
 * Copying the object will not clone the object, but merely copy the handle. Use
 * the clone function to fully copy the object
 *
 * RESOURCE ALLOCATION
 * no external resource are used
 *
 */

#include <stdlib.h>

#include <iostream>
#include <vector>
#include <string>

#include "matfilebase.h"

namespace MatlabIO {

// The matfiledata class is a helper class for the
// matfile class. As the data stored in the file can
// be of different formats, this class stores the data
// and the type of data. It also supports swapping the
// bytes of the data and converting them into other 
// formats.
// 
// The class should make it easier to transport data from
// the matfile class to the matlabfile class.

class matfile;
class matfiledata;

class matfiledata : public matfilebase {

	// make matfile a friend class so it can directly read and write
	// data into the memory managed by this object.
	friend class matfile;
  
	// structure definitions
  private:
	struct mxdata 
	{
		void	*dataptr_;	// Store the data to put in the matfile
		bool	owndata_;   // Do we own the data
		int	bytesize_;	// Size of the data in bytes
		mitype	type_;		// The type of the data
		int	ref_;		// reference counter
	};    

  // data objects	
  private:
	mxdata *m_;
	void *ptr_;
	void clearptr();
  
  // functions  
  public:
	matfiledata();
	~matfiledata();

	matfiledata(mitype type);

	matfiledata(const matfiledata &m); // copy constructor
	matfiledata& operator= (const matfiledata &m); // assignment

	// clear() will remove any databuffer and emtpty the object
	// After calling this function a new buffer can be created
	void clear();
	
	// newdatabuffer() will clear the object and will initiate a new 
	// buffer
	void newdatabuffer(int bytesize,mitype type);
	// void extdatabuffer(void *databuffer, int bytesize, mitype type);

	
	// clone the current object
	// i.e create a new databuffer and copy the actual data
	//     the normal assignment operator only copies the pointer
	matfiledata clone();
			
	// get/set type information
	// setting type information using type()
	// will not result in casting the data
	// contained in the databuffer
	
	void 	type(mitype type);
	mitype 	type();  

	// get size information.
	int size();			// size in elements
	int bytesize();		// size in bytes
	int elsize();			// size of the elements in the array
	int elsize(mitype type); // element size of a type

	// Direct access to data
	void getdata(void *dataptr,int bytesize);
	void putdata(void *dataptr,int bytesize,mitype type);
	
	// copying and casting templates	
	
	// copy and cast the data in a user defined memory space
	// dataptr and size specify the data block and the number of elements
	// that can be stored in this data block.
	template<class T> void getandcast(T *dataptr,int size);
	template<class T> void getandcast(T **dataptr,int dim1, int dim2);
	template<class T> void getandcast(T ***dataptr,int dim1, int dim2, int dim3);
	template<class T> void putandcast(T *dataptr,int size,mitype type);
	template<class T> void putandcast(T **dataptr,int dim1, int dim2, mitype type);
	template<class T> void putandcast(T ***dataptr,int dim1, int dim2, int dim3, mitype type);


	// For smaller arrays use the STL and put the data in a vector. These
	// vectors are copied and hence are less efficient. However using STL
	// there is no need to do memory management

	template<class T> void getandcastvector(std::vector<T> &vec);
	template<class T> void putandcastvector(std::vector<T> &vec,mitype type);

	template<class ITERATOR> void putandcast(ITERATOR is,ITERATOR ie,mitype type);


	// Access functions per element. 
	template<class T> T getandcastvalue(int index);
	template<class T> void putandcastvalue(T value,int index);

	// string functions	
	// support functions for reading and writing fieldnames and matrixnames
	// A struct arrray, can have multiple fields, hence an array of strings
	// needs to be read or written. Matlab stores string arrays differently
	// in comparison to a single string, hence the two different types of access
	// functions.
	
	std::string		 getstring();
	void 			 putstring(std::string str);
	std::vector<std::string> getstringarray(int strlength);
	int 			 putstringarray(std::vector<std::string>);
	
	// reorder will reorder the data in the datafield according to the indices
	// specified.
	
	matfiledata reorder(const std::vector<int> &newindices);
	matfiledata reorder(int *newindices,int size);

	// cast the data to a different numeric format
	matfiledata castdata(mitype type);
	
  protected:
	// This function should be used with care as destroying the object
	// will free the databuffer. A similar effect has clearing or
	// initiating a new buffer. 
	void *databuffer();
	
	void ptrset(void *ptr);
	void ptrclear();

};


////////////////////////////////////////
////// TEMPLATE FUNCTIONS///////////////
////////////////////////////////////////


template<class T> void matfiledata::getandcast(T *dataptr,int dsize)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
    
    if (databuffer() == 0) return;
    if (dataptr  == 0) return;
	if (dsize == 0) return;
	if (size() == 0) return;
    if (dsize > size()) dsize = size();	// limit casting and copying to amount of data we have		
	
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {dataptr[p] = static_cast<T>(ptr[p]); }}
	   break;	
        default:
           throw unknown_type();
    }
}


template<class T> void matfiledata::getandcast(T **dataptr,int dim1, int dim2)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
    
    if (databuffer() == 0) return;
    if (dataptr  == 0) return;
	if (dim1 == 0) return;
	if (dim2 == 0) return;
	if (size() == 0) return;
	
	// limit casting and copying to amount of data we have	
    if ((dim1*dim2) > size()) dim2 = size()/dim1;		
	if (dim2 < 1) dim2 = 1;
	if (dim1 > size()) dim1 = size();
	
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
		 int p,q,s;
		 s = 0; for(p=0;p<dim2;p++) for(q=0;q<dim1;q++) {dataptr[p][q] = static_cast<T>(ptr[s++]); }}
	   break;	
        default:
           throw unknown_type();
    }
}


template<class T> void matfiledata::getandcast(T ***dataptr,int dim1, int dim2, int dim3)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
    
    if (databuffer() == 0) return;
    if (dataptr  == 0) return;
	if (dim1 == 0) return;
	if (dim2 == 0) return;
	if (dim3 == 0) return;
	if (size() == 0) return;
	
	// limit casting and copying to amount of data we have	
    if ((dim1*dim2*dim3) > size()) dim3 = size()/(dim1*dim2);		
	if (dim3 < 1) dim3 = 1;
    if ((dim1*dim2) > size()) dim2 = size()/dim1;		
	if (dim2 < 1) dim2 = 1;
	if (dim1 > size()) dim1 = size();
	
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
		 int p,q,r,s;
		 s = 0; for(p=0;p<dim3;p++) for(q=0;q<dim2;q++) for(r=0;r<dim1;r++) {dataptr[p][q][r] = static_cast<T>(ptr[s++]); }}
	   break;	
        default:
           throw unknown_type();
    }
}


template<class T> void matfiledata::getandcastvector(std::vector<T> &vec)
{

    // This function copies and casts the data into a vector container
   
    int dsize = size();
    vec.resize(dsize);
	
	if (databuffer() == 0) { vec.resize(0); return; }
    if (size() == 0) { vec.resize(0); return; };
			    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;		
#ifdef JGS_MATLABIO_USE_64INTS
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64*>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;	
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {vec[p] = static_cast<T>(ptr[p]); }}
	   break;
        default:
           throw unknown_type();           	   
    }
}


template<class T> T matfiledata::getandcastvalue(int index)
{
    // direct access to the data

    T val = 0;
    if (databuffer() == 0) throw out_of_range();
    if (index >= size()) throw out_of_range();
    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64*>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
#endif
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
	case miDOUBLE: 
	  { double *ptr = static_cast<double *>(databuffer()); val = static_cast<T>(ptr[index]);}
	   break;
        default:
           throw unknown_type();           
    }
    return(val);
}



// functions inserting data


template<class T> void matfiledata::putandcast(T *dataptr,int dsize,mitype dtype)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
   
    clear(); 
    if (dataptr  == 0) return;
    
    newdatabuffer(dsize*elsize(dtype),dtype);
    
    switch (dtype)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<signed char>(dataptr[p]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<unsigned char>(dataptr[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<signed short>(dataptr[p]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<unsigned short>(dataptr[p]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<int32_t>(dataptr[p]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<u_int32_t>(dataptr[p]); }}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   		
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<int64>(dataptr[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<uint64>(dataptr[p]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<float>(dataptr[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) { ptr[p] = static_cast<double>(dataptr[p]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}


template<class T> void matfiledata::putandcast(T **dataptr,int dim1, int dim2 ,mitype dtype)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
   
    clear(); 
    if (dataptr  == 0) return;
    
    newdatabuffer(dim1*dim2*elsize(dtype),dtype);
    
    switch (dtype)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<signed char>(dataptr[p][q]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<unsigned char>(dataptr[p][q]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<signed short>(dataptr[p][q]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<signed short>(dataptr[p][q]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<int32_t>(dataptr[p][q]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<u_int32_t>(dataptr[p][q]); }}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   		
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<int64>(dataptr[p][q]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<uint64>(dataptr[p][q]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<signed char>(dataptr[p][q]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim2;p++) for(int q=0;q<dim1;q++) { ptr[s++] = static_cast<signed char>(dataptr[p][q]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}

template<class T> void matfiledata::putandcast(T ***dataptr,int dim1, int dim2, int dim3 ,mitype dtype)
{
    // This function copies and casts the data in the matfilebuffer into
    // a new buffer specified by dataptr (address of this new buffer) with
    // size size (number of elements in this buffer)
   
    clear(); 
    if (dataptr  == 0) return;
    
    newdatabuffer(dim1*dim2*dim3*elsize(dtype),dtype);
    
    switch (dtype)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed char>(dataptr[p][q][r]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<unsigned char>(dataptr[p][q][r]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed short>(dataptr[p][q][r]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<unsigned short>(dataptr[p][q][r]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<int32_t>(dataptr[p][q][r]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<u_int32_t>(dataptr[p][q][r]); }}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   		
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed char>(dataptr[p][q][r]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed char>(dataptr[p][q][r]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed char>(dataptr[p][q][r]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
       int s;
	     s = 0; for(int p=0;p<dim3;p++) for(int q=0;q<dim2;q++) for(int r=0;r<dim2;r++) { ptr[s++] = static_cast<signed char>(dataptr[p][q][r]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}


template<class T> void matfiledata::putandcastvector(std::vector<T> &vec,mitype type)
{
    clear();
    
    int dsize = static_cast<int>(vec.size());
	
	if (dsize == 0) return;
    newdatabuffer(dsize*elsize(type),type);
    
    switch (type)
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<signed char>(vec[p]); }}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<unsigned char>(vec[p]); }}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<signed short>(vec[p]); }}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<unsigned short>(vec[p]); }}
	   break;   
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<int32_t>(vec[p]); }}
	   break;   
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<u_int32_t>(vec[p]); }}
	   break;	
#ifdef JGS_MATLABIO_USE_64INTS	   	
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<int64>(vec[p]); }}
	   break;   
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<uint64>(vec[p]); }}
	   break;	
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<float>(vec[p]); }}
	   break;   	
	case miDOUBLE: 
	   { double *ptr = static_cast<double *>(databuffer());
	     for(int p=0;p<dsize;p++) {ptr[p] = static_cast<double>(vec[p]); }}
	   break;	
        default:
           throw unknown_type();              
    }
}


template<class T> void matfiledata::putandcastvalue(T val,int index)
{
    if (index >= size()) throw out_of_range();
    
    switch (type())
    {
	case miINT8: 
	   { signed char *ptr = static_cast<signed char *>(databuffer()); ptr[index] = static_cast<signed char>(val);}
	   break;
	case miUINT8: case miUTF8:
	   { unsigned char *ptr = static_cast<unsigned char *>(databuffer()); ptr[index] = static_cast<unsigned char>(val);}
	   break;
	case miINT16: 
	   { signed short *ptr = static_cast<signed short *>(databuffer()); ptr[index] = static_cast<signed short>(val);}
	   break;
	case miUINT16: case miUTF16:
	   { unsigned short *ptr = static_cast<unsigned short *>(databuffer()); ptr[index] = static_cast<unsigned short>(val);}
	   break;
	case miINT32: 
	   { int32_t *ptr = static_cast<int32_t *>(databuffer()); ptr[index] = static_cast<int32_t>(val);}
	   break;
	case miUINT32: case miUTF32:
	   { u_int32_t *ptr = static_cast<u_int32_t *>(databuffer()); ptr[index] = static_cast<u_int32_t>(val);}
	   break;
#ifdef JGS_MATLABIO_USE_64INTS	   
	case miINT64: 
	   { int64 *ptr = static_cast<int64 *>(databuffer()); ptr[index] = static_cast<int64>(val);}
	   break;
	case miUINT64: 
	   { uint64 *ptr = static_cast<uint64 *>(databuffer()); ptr[index] = static_cast<uint64>(val);}
	   break;
#endif	   
	case miSINGLE: 
	   { float *ptr = static_cast<float *>(databuffer()); ptr[index] = static_cast<float>(val);}
	   break;
	case miDOUBLE: 
	  { double *ptr = static_cast<double *>(databuffer()); ptr[index] = static_cast<double>(val);}
	   break;
        default:
           throw unknown_type();           
    }
}



template<class ITERATOR> 
void matfiledata::putandcast(ITERATOR is, ITERATOR ie, mitype type)
{
  clear();
  
  // determine size
  ITERATOR it = is;
  int dsize = 0;
  while(it != ie) { dsize++; ++it; } 
  
  
	if (dsize == 0) return;
    
  newdatabuffer(dsize*elsize(type),type);
    
  switch (type)
  {
    case miINT8: 
      { 
        int p = 0;
        signed char *ptr = static_cast<signed char *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<signed char>(*is); ++is; }
      }
      break;
    case miUINT8: case miUTF8:
      { 
        int p = 0;
        unsigned char *ptr = static_cast<unsigned char *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<unsigned char>(*is); ++is; }
      }
      break;
    case miINT16: 
      { 
        int p = 0;
        signed short *ptr = static_cast<signed short *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<signed short>(*is); ++is; }
      }
      break;
    case miUINT16: case miUTF16:
      { 
        int  p = 0;
        unsigned short *ptr = static_cast<unsigned short *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<unsigned short>(*is); ++is; }
      }
      break;
	case miINT32: 
      { 
        int p = 0;
        signed int *ptr = static_cast<signed int *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<signed int>(*is); ++is; }
      }
      break;
	case miUINT32: case miUTF32:
      { 
        int p = 0;
        unsigned int *ptr = static_cast<unsigned int *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<unsigned int>(*is); ++is; }
      }
      break;
#ifdef JGS_MATLABIO_USE_64INTS	   	
	case miINT64: 
      { 
        int p = 0;
        int64 *ptr = static_cast<int64 *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<int64>(*is); ++is; }
      }
      break;
	case miUINT64: 
      { 
        int p = 0;
        uint64 *ptr = static_cast<uint64 *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<uint64>(*is); ++is; }
      }
      break;
#endif	   
	case miSINGLE: 
      { 
        int p = 0;
        float *ptr = static_cast<float *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<float>(*is); ++is; }
      }
      break;
  case miDOUBLE: 
      { 
        int p = 0;
        double *ptr = static_cast<double *>(databuffer());
        while(is != ie) {ptr[p++] = static_cast<double>(*is); ++is; }
      }
      break;
  default:
      throw unknown_type();              
  }
}

} // end namespace

#endif
