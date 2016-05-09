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

#include <Core/State/StateEngine.h>
#include <Core/State/StateLabeledMultiOption.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

//////////////////////////////////////////////////////////////////////////
// Class StateLabeledMultiOptionPrivate
//////////////////////////////////////////////////////////////////////////

class StateLabeledMultiOptionPrivate
{
  // -- Constructors --
public:
  StateLabeledMultiOptionPrivate( const std::vector< std::string >& value, 
    const std::string& option_list );
  StateLabeledMultiOptionPrivate( const std::vector< std::string >& value, 
    const std::vector< OptionLabelPair >& labeled_option_list );

public:
  void import_options_from_string( const std::string& option_list );
  int find_option( const std::string& value );

public:
  std::vector< std::string > value_;
  std::vector< OptionLabelPair > option_list_;
};

StateLabeledMultiOptionPrivate::StateLabeledMultiOptionPrivate( 
  const std::vector< std::string >& value, const std::string& option_list )
{
  this->import_options_from_string( option_list );
  size_t num_selections = value.size();
  for ( size_t i = 0; i < num_selections; ++i )
  {
    std::string selection = Core::StringToLower( value[ i ] );
    if ( this->find_option( selection ) != - 1 )
    {
      this->value_.push_back( selection );
    }
    else
    {
      CORE_THROW_LOGICERROR( "Option '" + selection + "' not in the option list" );
    }
  }
}

StateLabeledMultiOptionPrivate::StateLabeledMultiOptionPrivate( 
  const std::vector< std::string >& value, 
  const std::vector< OptionLabelPair >& labeled_option_list )
{
  size_t num_options = labeled_option_list.size();
  this->option_list_.resize( num_options );
  for ( size_t i = 0; i < num_options; i++ )
  {
    this->option_list_[ i ].first = Core::StringToLower( labeled_option_list[ i ].first );
    this->option_list_[ i ].second = labeled_option_list[ i ].second;
  }

  size_t num_selections = value.size();
  for ( size_t i = 0; i < num_selections; ++i )
  {
    std::string selection = Core::StringToLower( value[ i ] );
    if ( this->find_option( selection ) != - 1 )
    {
      this->value_.push_back( selection );
    }
    else
    {
      CORE_THROW_LOGICERROR( "Option '" + selection + "' not in the option list" );
    }
  }
}

void StateLabeledMultiOptionPrivate::import_options_from_string( const std::string& option_list )
{
  if ( option_list.size() == 0 )
  {
    this->option_list_.clear();
    return;
  }
  
  std::vector< std::string > option_label_vec = Core::SplitString( option_list, "|" );
  size_t num_options = option_label_vec.size();
  this->option_list_.resize( num_options );
  for ( size_t i = 0; i < num_options; i++ )
  {
    std::vector< std::string > option_label_pair = 
      Core::SplitString( option_label_vec[ i ], "=" );
    if ( option_label_pair.size() < 2 )
    {
      CORE_THROW_INVALIDARGUMENT( option_label_vec[ i ] + 
        " is not a valid option-label pair" );
    }
    this->option_list_[ i ].first = Core::StringToLower( option_label_pair[ 0 ] );
    this->option_list_[ i ].second = option_label_pair[ 1 ];
  }
}

int StateLabeledMultiOptionPrivate::find_option( const std::string& value )
{
  int num_options = static_cast< int >( this->option_list_.size() );
  for ( int i = 0; i < num_options; i++ )
  {
    if ( this->option_list_[ i ].first == value ||
      Core::StringToLower( this->option_list_[ i ].second ) == value )
    {
      return i;
    }
  }

  return -1;
}

//////////////////////////////////////////////////////////////////////////
// Class StateLabeledMultiOption
//////////////////////////////////////////////////////////////////////////

StateLabeledMultiOption::StateLabeledMultiOption( const std::string& stateid, 
  const std::vector< std::string >& default_value, const std::string& labeled_option_list ) :
  StateBase( stateid ),
  private_( new StateLabeledMultiOptionPrivate( default_value, labeled_option_list ) )
{
}

StateLabeledMultiOption::StateLabeledMultiOption( const std::string& stateid, 
  const std::vector< std::string >& default_value, 
  const std::vector< OptionLabelPair >& labeled_option_list ) :
  StateBase( stateid ),
  private_( new StateLabeledMultiOptionPrivate( default_value, labeled_option_list ) )
{
}

StateLabeledMultiOption::~StateLabeledMultiOption()
{
}

std::string StateLabeledMultiOption::export_to_string() const
{
  return Core::ExportToString( this->private_->value_ );
}

bool StateLabeledMultiOption::import_from_string( const std::string& str, 
  ActionSource source )
{
  std::vector< std::string > value;
  if ( !Core::ImportFromString( str, value ) )
  {
    return false;
  }

  return this->set( value, source );
}

void StateLabeledMultiOption::export_to_variant( Variant& variant ) const
{
  variant.set( this->private_->value_ );
}

bool StateLabeledMultiOption::import_from_variant( Variant& variant, 
  Core::ActionSource source)
{
  std::vector< std::string > value;
  if ( !variant.get( value ) )
  {
    return false;
  }

  return this->set( value, source );
}

bool StateLabeledMultiOption::validate_variant( Variant& variant, std::string& error )
{
  std::vector< std::string > value;
  if ( !( variant.get( value ) ) )
  {
    error = "Cannot convert the value '" + variant.export_to_string() + "'";
    return false;
  }

  size_t num_selections = value.size();
  for ( size_t i = 0; i < num_selections; ++i )
  {
    if ( this->private_->find_option( StringToLower( value[ i ] ) ) == -1 )
    {
      error = "Option '" + value[ i ] + "' is not a valid option";
      return false;
    }
  }
  
  error = "";
  return true;
}

void StateLabeledMultiOption::set_option_list(  const std::vector< OptionLabelPair >& option_list )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
  
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::GetMutex() );

  size_t num_options = option_list.size();
  this->private_->option_list_.resize( num_options );
  for ( size_t i = 0; i < num_options; i++ )
  {
    this->private_->option_list_[ i ].first = Core::StringToLower( option_list[ i ].first );
    this->private_->option_list_[ i ].second = option_list[ i ].second;
  }

  std::vector< std::string > old_value = this->private_->value_;
  bool changed = false;
  this->private_->value_.clear();
  for ( size_t i = 0; i < old_value.size(); ++i )
  {
    if ( this->private_->find_option( old_value[ i ] ) != -1 )
    {
      this->private_->value_.push_back( old_value[ i ] );
    }
    else
    {
      changed = true;
    }
  }
  
  lock.unlock();
  
  if ( this->signals_enabled() )
  {
    this->optionlist_changed_signal_();
    if ( changed )
    {
      this->value_changed_signal_( this->private_->value_, ActionSource::NONE_E );
      this->state_changed_signal_();
    }
  }
}

const std::vector< OptionLabelPair >& StateLabeledMultiOption::get_option_list() const
{
  return this->private_->option_list_;
}

const std::vector< std::string >& StateLabeledMultiOption::get() const
{
  return this->private_->value_;
}

bool StateLabeledMultiOption::set( const std::vector< std::string >& input_value, 
                  Core::ActionSource source )
{
  // NOTE: State variables can only be set from the application thread
  ASSERT_IS_APPLICATION_THREAD_OR_INITIALIZING();
  
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock( StateEngine::Instance()->get_mutex() );

  std::vector< std::string > selections;
  for ( size_t i = 0; i < input_value.size(); ++i )
  {
    std::string selection = StringToLower( input_value[ i ] );
    int index = this->private_->find_option( selection );
    if ( index != -1 )
    {
      // NOTE: input_value can be either an option or its label, so it's necessary to set
      // the value based on the index
      selections.push_back( this->private_->option_list_[ index ].first );
    }
    else
    {
      break;
    }
  }
  
  if ( selections.size() != input_value.size() )
  {
    if ( source == ActionSource::INTERFACE_WIDGET_E )
    {
      lock.unlock();
      // NOTE: This is a special case in which the option requested by the
      // interface does not exist and hence the value may be out of sync and
      // hence needs to be set to the correct value. Hence we generate the
      // signal that indicates that the state has changed anyway.

      // Any other sources are fine as they do not reflect a different value
      // and are validated before the code can reach this point.
      if ( this->signals_enabled() )
      {     
        this->value_changed_signal_( this->private_->value_, source );
        this->state_changed_signal_();
      }
    }

    return false;
  }

  if ( this->private_->value_ == selections )
  {
    return true;
  }
  
  this->private_->value_ = selections;
  lock.unlock();

  if ( this->signals_enabled() )
  { 
    this->value_changed_signal_( this->private_->value_, source );
    this->state_changed_signal_();
  }
  
  return true;
}

} // end namespace Core
