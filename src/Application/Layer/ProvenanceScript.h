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

#ifndef APPLICATION_LAYER_PROVENANCESCRIPT_H
#define APPLICATION_LAYER_PROVENANCESCRIPT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <map>

// Core includes
#include <Core/Utils/EnumClass.h>

// Application includes
#include <Application/Provenance/ProvenanceStep.h>

namespace Seg3D
{

typedef std::map< ProvenanceID, std::string > ProvenanceIDLayerIDMap;

CORE_ENUM_CLASS
(
  ScriptStatus,
  VALID_E,
  INCOMPLETE_E,
  INVALID_E
)

/// GENERATESCRIPTFROMPROVENANCETRAIL:
/// Generate python script from the given provenance trail.
/// Returns ScriptStatus::VALID_E if successful, ScriptStatus::INCOMPLETE_E if successful
/// but some inputs are missing, ScriptStatus::INVALID_E if failed.
ScriptStatus GenerateScriptFromProvenanceTrail( ProvenanceTrailHandle prov_trail, 
      std::string& script, ProvenanceIDLayerIDMap& inputs );

} // end namespace Seg3D

#endif
