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

// Application Includes
#include <Application/Application/Application.h>

// Boost Includes
#include<boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

// Core includes
#include <Utils/Core/Log.h>
#include <Utils/Core/LogHistory.h>

namespace Seg3D {

Application::Application()
{
  // The event handler needs to be started manually
  // This event handler will execute all the functions
  // that are send to it on the main application thread.
  start_eventhandler();
}

  // Singleton instance
  Utils::Singleton<Application> Application::instance_;
    
  //This map stores the parameters that were set when Seg3D was started.  
  static std::map <std::string, std::string> parameters;


  //This is a function to check parameters.  
  //This avoids accidentally putting data into the map that we dont want
  std::string
  Application::checkCommandLineParameter( const std::string &key )
  {
    if (parameters.find(key) == parameters.end()) {
      return "0";
    }
    else {
      return parameters[key];
    }
  }

  //This function sets parameters in the parameters map.
  void
  Application::setParameter( const std::string &key, const std::string &val )
  {
    parameters[key] = val;
  }
  
  // Function for parsing the command line parameters
  void
  Application::parse_command_line_parameters( int argc, char **argv)
  {
    int count_ = 1;  // start at 1 because the filename/path counts as 0
    std::string key;
    std::string value;
    
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> seperator(":-=|;");
    
    // parse through the command line arguments
    while (count_ < argc) 
    {
      std::string param(argv[count_]);
      tokenizer tokens(param, seperator);
      std::vector<std::string> param_vector_;
      
      for (tokenizer::iterator tok_iter = tokens.begin();tok_iter != tokens.end(); ++tok_iter)
      {
        param_vector_.push_back(*tok_iter);
      } // end for loop
      
      if (param_vector_.size() == 2) {
        key = param_vector_[0];
        value = param_vector_[1];
      } // end if
      else 
      {
        key = param_vector_[0];
        value = "1";
      } // end else
      
      // output the parsed parameters to the log
      std::string parameter_number_ = boost::lexical_cast<std::string>(count_);
      SCI_LOG_MESSAGE("Parameter " + parameter_number_ + " - Key: " + key + ", Value: " + value);
      
      setParameter(key, value);
      count_++;
    }  // end while
    
  }  // end parse_command_line_parameters
  
} // end namespace Seg3D
