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

// Application includes
#include <Core/State/StateIO.h>

// TinyXML includes
#include <Externals/tinyxml/tinyxml.h>

// Core includes
#include <Core/Utils/StringUtil.h>



namespace Core
{

StateIO::StateIO()
{
}

StateIO::~StateIO()
{
}

bool StateIO::export_to_file( boost::filesystem::path path, std::vector< std::string >& state_list,
  bool project_file)
{
  // XML declaration and version number
  TiXmlDocument doc;  
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );  
  doc.LinkEndChild( decl );  

  // Set Seg3D2 as our root
  TiXmlElement * root = new TiXmlElement( "Seg3D2" );  
  doc.LinkEndChild( root );  
 
  // We will use our statehandler_id as the parent for its state values
  TiXmlElement* branch = new TiXmlElement( state_list[ 0 ].c_str() );
  std::string node = state_list[ 0 ];
  root->LinkEndChild( branch ); 

  TiXmlElement* state_value;

  for( int i = 1; i < ( static_cast< int >( state_list.size() ) - 1 ); ++i )
  {
    if( node == state_list[ i ] )
    {
      branch = new TiXmlElement( state_list[ i + 1 ].c_str() );
      root->LinkEndChild( branch );
      node = state_list[ i + 1 ];
      i++;
      continue;
    }
    std::vector< std::string > state_value_as_string_vector = 
      SplitString( state_list[ i ], "*" );

    if( state_value_as_string_vector.size() == 3 )
    {
      state_value = new TiXmlElement( ( state_value_as_string_vector[ 0 ] + ":" +
        state_value_as_string_vector[ 1 ] ).c_str() );
      state_value->LinkEndChild( new TiXmlText( state_value_as_string_vector[ 2 ].c_str() ) );
    }
    else
    {
      state_value = new TiXmlElement( state_value_as_string_vector[ 0 ].c_str() );
      state_value->LinkEndChild( new TiXmlText( state_value_as_string_vector[ 1 ].c_str() ) );
    }
    branch->LinkEndChild( state_value );
    
  } // end state_list for loop

  std::string extension = ".xml";
  if( project_file ) 
  {
    extension = ".s3d";
  }
  
  doc.SaveFile( ( ( path ).string() + extension ).c_str() );

  return true;
}

bool StateIO::import_from_file( boost::filesystem::path path, std::vector< std::string >& state_list,
  bool project_file )
{

  std::string extension = ".xml";
  if( project_file ) 
  {
    extension = ".s3d";
  }
  
  // We will load in the file from the specified path and exit if the path is invalid
  TiXmlDocument doc( ( path.string() + extension ).c_str() );
  if ( !doc.LoadFile() ) 
    return false;

  TiXmlHandle hDoc( &doc );
  TiXmlElement* state_handler;
  TiXmlElement* state_value;
  TiXmlHandle hRoot(0);

  // We should have a valid root if not we will exit
  {
    state_handler = hDoc.FirstChildElement().Element();
    if ( !state_handler ) return false;
    hRoot = TiXmlHandle( state_handler );
  }

  // Now we are expecting to get the proper statehandler_id_
  {
    state_handler = hRoot.FirstChildElement().Element();
    if ( !state_handler ) return false;

    for( ; state_handler; state_handler = state_handler->NextSiblingElement() )
    {
      // Finally we import the actual state values from the XML and import them
      std::string state_handler_name = std::string( state_handler->Value() );
      state_list.push_back( state_handler_name );

      state_value = state_handler->FirstChildElement();
      if ( !state_value ) return false;
      for (  ; state_value; state_value = state_value->NextSiblingElement() )
      {
        std::string state_handler_name = std::string( state_value->Value() );
        std::string state_value_value = "";
        if( SplitString( state_handler_name, ":" ).size() == 2 )
          state_handler_name.replace( state_handler_name.find( ":" ), 1, "*" );
        if( state_value->GetText() != NULL )
        {
          state_value_value = std::string( state_value->GetText() );
        }
        state_list.push_back( state_handler_name + "*" + state_value_value );
      
      }
      state_list.push_back( state_handler_name );
    }
  }


  return true;
}


} // end namespace Core
