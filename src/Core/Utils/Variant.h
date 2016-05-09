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

#ifndef CORE_UTILS_VARIANT_H
#define CORE_UTILS_VARIANT_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL
#include <algorithm>
#include <string>
#include <typeinfo>

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

// Core
#include <Core/Utils/StringUtil.h>

namespace Core
{

class VariantBase;
typedef boost::shared_ptr< VariantBase > VariantBaseHandle;

class VariantBase
{
  // -- destructor --
public:
  virtual ~VariantBase();

  // -- converters for import and export --
public:
  // -- functions for accessing data --
  // EXPORT_TO_STRING
  /// export the contents of the parameter to string
  virtual std::string export_to_string() const = 0;

  // IMPORT_FROM_STRING
  /// import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str ) = 0;

  // INTERNAL_TYPE:
  /// Return a const reference to the type_info object representing
  /// the actual type of value currently stored by the Variant object.
  virtual const std::type_info& internal_type() const = 0;
};

// VARIANT:
// Parameter for an action.

// Forward declaration:
template< class T > class VariantT;

// Class definition:
template< class T >
class VariantT : public VariantBase
{

  // -- define handle --
public:
  typedef boost::shared_ptr< VariantT< T > > Handle;

  // -- constructor/destructor --
public:
  VariantT()
  {
  }

  VariantT( const T& default_value ) :
    value_( default_value )
  {
  }

  virtual ~VariantT()
  {
  }

  // -- access to value --
public:

  // Get access similar to the variant version
  bool get( T& value )
  {
    value = value_;
    return true;
  }

  // Set access similar to the variant version
  void set( const T& value )
  {
    value_ = value;
  }

  // EXPORT_TO_STRING
  /// export the contents of the parameter to string
  virtual std::string export_to_string() const
  {
    return ExportToString( this->value_ );
  }

  // IMPORT_FROM_STRING
  /// import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str )
  {
    return ImportFromString( str, this->value_ );
  }

  // INTERNAL_TYPE:
  /// Return a const reference to the type_info object representing
  /// the actual type of value currently stored by the Variant object.
  virtual const std::type_info& internal_type() const
  {
    return typeid( T );
  }

private:
  /// The actual value
  T value_;
};



// VARIANT:
// Parameter for an action.

// Forward declaration:
class Variant;


// Class definition:
class Variant : public VariantBase
{

  // -- define handle --
public:
  typedef boost::shared_ptr< Variant > Handle;

  // -- constructor/destructor --
public:
  // constructor of untyped parameter
  Variant();

  // constructor of typed version
  template< class T >
  Variant( const T& default_value )
  {
    this->typed_value_ = VariantBaseHandle( new VariantT< T > ( default_value ) );
  }

  // copy constructor
  Variant( const Variant& variant )
  {
    this->typed_value_.reset();
    this->string_value_ = variant.export_to_string();
  }

  // desctructor
  virtual ~Variant();

  // assignment operator
  Variant& operator=( Variant& variant )
  {
    this->typed_value_.reset();
    this->string_value_ = variant.export_to_string();   
    return *this;
  }

  // -- get/set data --
public:
  // SET
  /// Set the value using a typed version of the parameter
  template< class T >
  void set( const T& value )
  {
    // Set the value typed
    typed_value_ = VariantBaseHandle( new VariantT< T > ( value ) );
    // Clear string version
    string_value_.clear();
  }

  // GET_VALUE
  /// Get the value from string or typed value. If a typed one is available
  /// use that one.
  template< class T >
  bool get( T& value )
  {
    // If a typed version exists
    if ( typed_value_.get() )
    {
      // if the typed version exists, use that one
      // use a dynamic cast to ensure that the type is correct
      VariantT<T>* variant_ptr = dynamic_cast< VariantT< T >* > ( typed_value_.get() );
      if ( variant_ptr == 0 )
      {
        if ( !( ImportFromString( typed_value_->export_to_string(), value ) ) )
        {
          return false;
        }

        return true;
      }

      variant_ptr->get( value );
      return true;
    }
    else
    {
      // Generate a new typed version. So it is only converted once
      VariantT< T >* variant_ptr = new VariantT< T > ;
      typed_value_ = VariantBaseHandle( variant_ptr );
      if ( !( typed_value_->import_from_string( string_value_ ) ) )
      {
        return false;
      }

      variant_ptr->get( value );
      return true;
    }
  }

  // VALIDATE_TYPE
  /// Check and convert to a certain type, but do not return the value
  /// This function is intended for validating the action by forcing the
  /// contained value to be converted to a certain type
  template< class T >
  bool validate_type()
  {
    T dummy;
    return this->get( dummy );
  }

  // INTERNAL_TYPE:
  /// Return a const reference to the type_info object representing
  /// the actual type of value currently stored by the Variant object.
  virtual const std::type_info& internal_type() const
  {
    if ( this->typed_value_ )
    {
      return this->typed_value_->internal_type();
    }
    return typeid( std::string );
  }

  // -- functions for accessing data --
public:

  // EXPORT_TO_STRING
  /// export the contents of the parameter to string
  virtual std::string export_to_string() const;

  // IMPORT_FROM_STRING
  /// import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str );

private:

  // Typed version
  VariantBaseHandle typed_value_;

  // String version
  std::string string_value_;

};

// IO Functions
std::string ExportToString( const Variant& variant );
bool ImportFromString( const std::string& str, Variant& variant );

} // namespace Core

#endif
