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

#ifdef _MSC_VER
#pragma warning( disable: 4244 4267 )
#endif

// STL includes
#include <iostream>
#include <string>

// boost includes
#include <boost/filesystem.hpp>
#include <boost/preprocessor.hpp>
#include <boost/algorithm/string.hpp>

// Core includes
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/FileUtil.h>
#include <Core/Utils/LogStreamer.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Math/MathFunctions.h>
#include <Core/Application/Application.h>
#include <Core/Log/RolloverLogFile.h>

#include <Core/LargeVolume/LargeVolumeConverter.h>

void printUsage() {
  std::cout << "USAGE: " << Core::Application::Instance()->GetUtilName()
            <<  " first_file output_volume [OPTIONS]" << std::endl;
  std::cout << "Volume must be an image stack."
            << " The volume will be processed into bricks of decreasing level of detail, starting from full resolution." << std::endl << std::endl;
  std::cout << "Mandatory arguments:" << std::endl;
  std::cout << "  first_file                   - First file in image stack file series." << std::endl;
  std::cout << "  output_volume                - Path to directory in which volume is stored as bricks." << std::endl
            << "                                 Directory will be output as output_volume.s3dvol." << std::endl << std::endl;
  std::cout << "Dataset parameters (optional):" << std::endl;
  std::cout << "  --origin=VECTOR              - Origin of volume in space coordinates, default is 0,0,0." << std::endl;
  std::cout << "  --spacing=VECTOR             - Spacing of volume in space coordinates, default is 1,1,1." << std::endl;
  std::cout << "  --bricksize=VECTOR | SCALAR  - Brick size, default is 256,256,256." << std::endl
            << "                                 Size of bricks can be set with single number (--bricksize=512 for 512,512,512 brick)." << std::endl;
  std::cout << "  --overlap=SCALAR             - Overlap betweeen the bricks, default is 1." << std::endl;
  std::cout << "  --nodownsample=CHAR          - Do not downsample in given direction (x,y, or z)." << std::endl << std::endl;
  std::cout << "Tool parameters (optional):" << std::endl;
  std::cout << "  --maxgb=SCALAR               - Maximum number of GB to use for conversion, default is based on available memory." << std::endl;
  std::cout << "  --silent                     - Do not wait for user input to continue." << std::endl;
}

// TODO: header full of useful constants (e.g. default brick size)

int main( int argc, char **argv )
{
  Core::Application::SetUtilName("CreateLargeVolume");
  
  // -- Parse the command line parameters --
  if ( argc == 2 )
  {
    Core::Application::Instance()->parse_command_line_parameters( argc, argv, 0 );
    
    // Get version information
    
    // Print version help
    if ( Core::Application::Instance()->is_command_line_parameter( "version") )
    {
      // NOTE: This information is gathered by cmake from the top-level CMakeLists.txt file.
      std::cout << Core::Application::Instance()->GetApplicationName()
      << " version: " <<
      Core::Application::Instance()->GetVersion() << std::endl;
      return 0;
    }
    // Print usage help
    else
    {
      printUsage();
      return 0;
    }
  }
  
  // -- Parse the command line parameters --
  Core::Application::Instance()->parse_command_line_parameters( argc, argv, 2 );
  
  // -- Send message to revolving log file --
  // Logs messages in response to Log::Instance()->post_log_signal_
  Core::RolloverLogFile event_log( Core::LogMessageType::ALL_E );
  
  // -- Log application information --
  Core::Application::Instance()->log_start();
  
  // parameters
  
  // -- Input file --
  boost::filesystem::path first_file( Core::Application::Instance()->get_argument( 0 ) );
  
  if (!boost::filesystem::exists( first_file ))
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR("Input file '" + first_file.string() + "' does not exist.");
    return -1;
  }

  if (! Core::FileUtil::CheckExtension( first_file, ".tif|.tiff|.png|.jpg|.jpeg|.dcm|.dicom" ) )
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR("Input file needs to be a png, jpg, or tiff file.");
    return -1;
  }
  
  // -- Output directory --
  boost::filesystem::path output_dir( Core::Application::Instance()->get_argument( 1 ) );
  
  if (! Core::FileUtil::CheckExtension( output_dir, ".s3dvol" ) )
  {
    output_dir = boost::filesystem::path( output_dir.string() + ".s3dvol" );
  }
  
  if (boost::filesystem::exists(output_dir))
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR("Output directory '" + output_dir.string() + "' already exists, please delete directory before starting conversion.");
    return -1;
  }
  
  // -- spacing --
  Core::Vector spacing(1.0,1.0,1.0);
  std::string spacing_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "spacing" , spacing_string ) )
  {
    if (!Core::ImportFromString( spacing_string, spacing ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR(" Spacing needs to be a vector in the following format: --spacing=0.1,0.1,0.1");
      return -1;
    }
  }
  
  // -- origin --
  Core::Point origin(0.0,0.0,0.0);
  std::string origin_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "origin" , origin_string ) )
  {
    if (!Core::ImportFromString(origin_string, origin))
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Origin needs to be a vector in the following format: --origin=0.1,0.1,0.1");
      return -1;
    }
  }
  
  // -- brick size --
  Core::IndexVector brick_size( 256, 256, 256);
  std::string brick_size_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "bricksize" , brick_size_string ) )
  {
    if ( !Core::ImportFromString( brick_size_string, brick_size ) )
    {
      
      size_t single_brick_size;
      
      if (! Core::ImportFromString( brick_size_string, single_brick_size ) )
      {
        printUsage();
        CORE_PRINT_AND_LOG_ERROR("Brick size needs to be a number or a vector: --bricksize=512,512,512 or --bricksize=512");
        return -1;
      }
      
      brick_size = Core::IndexVector( single_brick_size, single_brick_size, single_brick_size );
    }
    
    if (! Core::IsPowerOf2( brick_size.x() ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Brick size needs to be a power of two.");
      return -1;
    }
    
    if (! Core::IsPowerOf2( brick_size.y() ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Brick size needs to be a power of two.");
      return -1;
    }
    
    if (! Core::IsPowerOf2( brick_size.z() ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Brick size needs to be a power of two.");
      return -1;
    }
  }
  
  size_t overlap = 1;
  std::string overlap_string;
  if ( Core::Application::Instance()->check_command_line_parameter( "overlap" , overlap_string ) )
  {
    if (!Core::ImportFromString( overlap_string, overlap ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Overlap needs to be a number.");
      return -1;
    }
  }
  
  // -- resample info --
  
  bool down_sample_x = true;
  bool down_sample_y = true;
  bool down_sample_z = true;
  
  std::string nodownsample;
  if ( Core::Application::Instance()->check_command_line_parameter( "nodownsample" , nodownsample ) )
  {
    if (! ( nodownsample == "x" || nodownsample == "y" || nodownsample == "z" ) )
    {
      printUsage();
      CORE_PRINT_AND_LOG_ERROR("Nodownsample needs to be x, y, or z.");
      return -1;
    }
    
    if ( nodownsample == "x" ) down_sample_x = false;
    if ( nodownsample == "y" ) down_sample_y = false;
    if ( nodownsample == "z" ) down_sample_z = false;
  }
  
  long long mem_limit = 0;
  if ( sizeof(void *) == 4 )
  {
    // For 32bit systems, do not exceed 1 GB of data usage.
    // On Windows, addressable space is 2 GB, hence this leaves enough space for the program itself.
    mem_limit = static_cast<long long>(1) << 30;
  }
  else
  {
    // For 64bit systems, try to get a lot of space.
    // Get total memory size and subtract 2 GB (for running program and operating system).
    long long mem_size = Core::Application::Instance()->get_total_physical_memory();
    mem_size -= static_cast<long long>(2) << 30;
#ifdef _APPLE_
    // On OS X, operating system uses far more resources than on Windows, hence reduce memory footprint.
    mem_size = static_cast<long long>( 0.5 * mem_size );
#else
    mem_size = static_cast<long long>( 0.7 * mem_size );
#endif
    
    // If less than 1 GB, use 1 GB.
    if (mem_size < ( static_cast<long long>(1) << 30) ) mem_size = static_cast<long long>(1) << 30;
    
    // Do not use more than 32 GB, unless explicity requested.
    mem_limit = Core::Min(static_cast<long long>(32) << 30, mem_size);
    
    // User specified amount.
    std::string mem_limit_string;
    if ( Core::Application::Instance()->check_command_line_parameter( "maxgb" , mem_limit_string ) )
    {
      int gbs;
      if (! Core::ImportFromString(mem_limit_string, gbs))
      {
        printUsage();
        CORE_PRINT_AND_LOG_ERROR("Please specify the number of GBs to use as conversion memory.");
        return -1;
      }
      if ( gbs < 1)
      {
        printUsage();
        CORE_PRINT_AND_LOG_ERROR("Please allow for at least 1 GB for conversion.");
        return -1;
      }
      
      mem_limit = static_cast<long long>(gbs) << 30;
    }
  }
  
  Core::LargeVolumeConverterHandle converter( new Core::LargeVolumeConverter());
  
  converter->set_output_dir( output_dir );
  converter->set_first_file( first_file );
  converter->set_schema_parameters( spacing, origin, brick_size, overlap );
  converter->get_schema()->enable_downsample( down_sample_x, down_sample_y, down_sample_z );
  converter->set_mem_limit( mem_limit );
  
  // Scan files and compute schema
  std::string error;
  if (! converter->run_phase1(error) )
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR(error);
    return -1;
  }
  
  Core::LargeVolumeSchemaHandle schema = converter->get_schema();
  
  std::cout << "--------------------------------------------------" << std::endl;
  std::cout << "Create large volume with the following parameters:" << std::endl;
  std::cout << "Input File Series:  " << first_file << std::endl;
  std::cout << "Output Directory:   " << output_dir << std::endl;
  std::cout << "Dimensions:         " << Core::ExportToString( schema->get_size() ) << std::endl;
  std::cout << "Spacing:            " << Core::ExportToString( schema->get_spacing() ) << std::endl;
  std::cout << "Origin:             " << Core::ExportToString( schema->get_origin() ) << std::endl;
  std::cout << "Brick Size:         " << Core::ExportToString( schema->get_brick_size() ) << std::endl;
  std::cout << "Overlap:            " << Core::ExportToString( schema->get_overlap() ) << std::endl;
  std::cout << "Resolution Levels:  " << Core::ExportToString( schema->get_num_levels() ) << std::endl;
  std::cout << "Memory Usage Limit: " << Core::ExportToString( mem_limit >> 30 ) << " GB" << std::endl;
  if (nodownsample.size())
  {
    std::cout << "No downsample:      " << nodownsample << std::endl;
  }
  
  for (size_t j = 0; j < schema->get_num_levels(); j++)
  {
    std::cout << "level " << Core::ExportToString( j ) <<":            ratio="
    << Core::ExportToString(  schema->get_level_downsample_ratio( j ) ) << " size="
    << Core::ExportToString( schema->get_level_size( j ) ) <<  std::endl;
  }
  
  if (! Core::Application::Instance()->is_command_line_parameter( "silent" ) )
  {
    std::cout << "Press enter to start conversion" << std::endl;
    std::cin.get();
  }
  
  std::cout << "== Downsampling and bricking of files == " << std::endl;
  if (! converter->run_phase2( error ) )
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR( error );
    return -1;
  }
  
  std::cout << "== Compressing bricks and optimizing brick files ==" << std::endl;
  
  if (! converter->run_phase3( error ) )
  {
    printUsage();
    CORE_PRINT_AND_LOG_ERROR( error );
    return -1;
  }
  
  std::cout << "== done ==" << std::endl;
  
  // Indicate a successful finish of the program
  Core::Application::Instance()->log_finish();
  return ( 0 );
}
