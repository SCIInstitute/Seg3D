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

#ifndef CORE_DATABLOCK_DATABLOCKMANAGER_H
#define CORE_DATABLOCK_DATABLOCKMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>


#include <Core/Utils/Singleton.h>
#include <Core/Utils/Lockable.h>
#include <Core/Utils/AtomicCounter.h>

namespace Core 
{

class DataBlockManagerPrivate;
typedef boost::shared_ptr< DataBlockManagerPrivate > DataBlockManagerPrivateHandle;

class DataBlockManager : public RecursiveLockable
{
  CORE_SINGLETON( DataBlockManager );

  // -- constructor --
private:
  /// NOTE: This one is private, use the singleton Instance() function to access this class
  DataBlockManager();
  
public:
  // REGISTER_DATABLOCK:
  /// This function needs to be called by the various datablock factories. So we can look up in
  /// this DataBlock manager whether a datablock has already been loaded (needed for the mask data)
  void register_datablock( DataBlockHandle data_block, 
    DataBlock::generation_type generation = -1 );

  // UNREGISTER_DATABLOCK:
  /// Needed by DataBlock to remove its entry from the database.
  void unregister_datablock( DataBlock::generation_type generation );

  // FIND_DATABLOCK:
  /// Find a datablock with a specific generation number.
  bool find_datablock( DataBlock::generation_type generation, DataBlockHandle& datablock );

  // CLEAR:
  /// Remove all the data blocks.
  void clear();

public:
  
  // GET_GENERATION_COUNT:
  /// Get the most recent generation number
  /// NOTE: Needed by the project manager when storing the project updates to disk. As within a 
  /// aproject for provenance and session reasons we cannot use an older generation. The last
  /// used number needs to be stored.
  DataBlock::generation_type get_generation_count();
  
  // SET_GENERATION_COUNT:
  /// Set the current count in the generation counter
  /// NOTE: This is needed when loading an existing project, as each project has its own generation
  /// count. 
  void set_generation_count( DataBlock::generation_type generation );

  // INCREMENT_GENERATION:
  /// Generate a new generation number for a datablock. This function is called through the
  /// DataBlock class whenever the data in a datablock is changed.
  DataBlock::generation_type increase_generation( DataBlock::generation_type old_generation );

private:
  DataBlockManagerPrivateHandle private_;
};

} // end namespace Core

#endif
