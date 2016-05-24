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

// STL includes
#include <vector>

// Boost includes
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Qt include
#include <QFont>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

// Interface includes
#include <Interface/Application/ProvenanceTreeModel.h>

namespace Seg3D
{

class ProvenanceTreeItem;
typedef boost::shared_ptr< ProvenanceTreeItem > ProvenanceTreeItemHandle;

class ProvenanceTreeItem
{
public:
  ProvenanceTreeItem( ProvenanceTreeItem* parent = 0 ) :
    parent_( parent ),
    row_( 0 ),
    prov_step_index_( 0 )
  {}

  QVariant data( int column )
  {
    switch ( column )
    {
    case 0:
      return this->timestamp_;
    case  1:
      return this->user_;
    case 2:
      return this->action_name_;
    default:
      return QVariant();
    }
  }

  void add_child( ProvenanceTreeItemHandle child )
  {
    this->children_.push_back( child );
    child->parent_ = this;
    child->row_ = static_cast< int >( this->children_.size() - 1 );
  }

  std::vector< ProvenanceTreeItemHandle > children_;
  ProvenanceTreeItem* parent_;
  
  // The index of this item in its parent
  int row_;

  QString timestamp_; // Column 0
  QString user_; // Column 1
  QString action_name_; // Column 2
  QString action_params_; // Qt::UserRole + 1

  // Provenance IDs of interest
  // Qt::UserRole 
  QString poi_; 

  // The index of the corresponding provenance step in the provenance trail
  // Qt::UserRole + 2
  size_t prov_step_index_; 
};

class ProvenanceTreeModelPrivate : public QObject
{
public:
  ProvenanceTreeModelPrivate( ProvenanceTreeModel* parent ) :
    QObject( parent ),
    root_item_( new ProvenanceTreeItem )
  {
    root_item_->timestamp_ = "Timestamp";
    root_item_->user_ = "User";
    root_item_->action_name_ = "Action";

    this->timestamp_formatter_.imbue( std::locale( std::locale::classic(),
      new boost::posix_time::time_facet( "%H:%M %m-%d-%y" ) ) );
  }

  const std::string format_timestamp( ProvenanceStep::timestamp_type time )
  {
    this->timestamp_formatter_.str( "" );
    this->timestamp_formatter_ << time;
    return this->timestamp_formatter_.str();
  }

  // -- Internal variables --
public:
  ProvenanceTreeItemHandle root_item_;
  std::stringstream timestamp_formatter_;
};

//////////////////////////////////////////////////////////////////////////
// Class ProvenanceTreeModel
//////////////////////////////////////////////////////////////////////////

ProvenanceTreeModel::ProvenanceTreeModel( QObject *parent ) :
  QAbstractItemModel( parent )
{
  this->private_ = new ProvenanceTreeModelPrivate( this );
}

ProvenanceTreeModel::~ProvenanceTreeModel()
{
}

static const std::string ExportTimeToString( ProvenanceStep::timestamp_type time )
{
  
}

void ProvenanceTreeModel::set_provenance_trail( ProvenanceTrailHandle prov_trail )
{
  typedef boost::date_time::c_local_adjustor< ProvenanceStep::timestamp_type > local_time_adjustor;
  
  this->beginResetModel();

  // == rebuild the model ==

  this->private_->root_item_->children_.clear();
  const size_t num_steps = prov_trail ? prov_trail->size() : 0;
  for ( size_t i = 0; i < num_steps; ++i )
  {
    ProvenanceStepHandle prov_step = prov_trail->at( i );
    const std::string& action_name = prov_step->get_action_name();
    const std::string& action_params = prov_step->get_action_params();
    const ProvenanceIDList& poi = prov_step->get_provenance_ids_of_interest();
    const std::string& user_id = prov_step->get_username();
    // Convert the timestamp from UTC to local time
    const ProvenanceStep::timestamp_type prov_local_time = 
      local_time_adjustor::utc_to_local( prov_step->get_timestamp() );

    if ( prov_step->get_action_name() == "Paint" )
    {
      const size_t start_step = i;
      ++i;
      while ( i < num_steps )
      {
        prov_step = prov_trail->at( i );
        if ( prov_step->get_action_name() != "Paint" || 
          prov_step->get_username() != user_id )
        {
          break;
        }
        ++i;
      }

      --i;
      // Collapse consecutive paint strokes
      if ( start_step != i )
      {
        ProvenanceTreeItem* parent_item = new ProvenanceTreeItem;
        this->private_->root_item_->add_child( ProvenanceTreeItemHandle( parent_item ) );
        parent_item->action_name_ = "Paint";
        parent_item->action_params_ = "Various...";
        parent_item->timestamp_ = QString::fromStdString( this->private_->format_timestamp(
          local_time_adjustor::utc_to_local( prov_trail->at( i )->get_timestamp() ) ) );
        parent_item->user_ = QString::fromStdString( user_id );
        parent_item->poi_ = QString::fromStdString( Core::ExportToString( 
          prov_trail->at( i )->get_provenance_ids_of_interest() ) );
        parent_item->prov_step_index_ = i;

        for ( size_t j = start_step; j <= i; ++j )
        {
          ProvenanceTreeItem* child_item = new ProvenanceTreeItem;
          parent_item->add_child( ProvenanceTreeItemHandle( child_item ) );
          child_item->action_name_ = "Paint";
          child_item->action_params_ = QString::fromStdString( 
            prov_trail->at( j )->get_action_params() );
          child_item->timestamp_ = QString::fromStdString( this->private_->format_timestamp(
            local_time_adjustor::utc_to_local( prov_trail->at( j )->get_timestamp() ) ) );
          child_item->user_ = QString::fromStdString( user_id );
          child_item->poi_ = QString::fromStdString( Core::ExportToString( 
            prov_trail->at( j )->get_provenance_ids_of_interest() ) );
          child_item->prov_step_index_ = j;
        }

        continue;
      } // end if ( start_step != i )
    } // end if
    
    // No need to collapse, create a normal top level item
    ProvenanceTreeItem* item = new ProvenanceTreeItem;
    this->private_->root_item_->add_child( ProvenanceTreeItemHandle( item ) );
    item->action_name_ = QString::fromStdString( action_name );
    item->action_params_ = QString::fromStdString( action_params );
    item->timestamp_ = QString::fromStdString( this->private_->format_timestamp( prov_local_time ) );
    item->user_ = QString::fromStdString( user_id );
    item->poi_ = QString::fromStdString( Core::ExportToString( poi ) );
    item->prov_step_index_ = i;
  } // end for
  
  this->endResetModel();
}

int ProvenanceTreeModel::columnCount( const QModelIndex& /*parent*/ ) const
{
  return 3;
}

QVariant ProvenanceTreeModel::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() ) return QVariant();

  ProvenanceTreeItem* item = static_cast< ProvenanceTreeItem* >( index.internalPointer() );
  switch ( role )
  {
  case Qt::DisplayRole:
    return item->data( index.column() );
  case Qt::FontRole:
    {
      QFont font;
      if ( item->children_.size() > 0 )
      {
        font.setBold( true );
      }
      return font;
    }
  case PID_OF_INTEREST_E:
    return item->poi_;
  case ACTION_PARAMS_E:
    return item->action_params_;
  case PROV_STEP_INDEX_E:
        return static_cast< qulonglong >( item->prov_step_index_ );
  case TIMESTAMP_E:
    return item->timestamp_;
  case USER_ID_E:
    return item->user_;
  case ACTION_NAME_E:
    return item->action_name_;
  default:
    return QVariant();
  }
}

QModelIndex ProvenanceTreeModel::index( int row, int column, const QModelIndex& parent ) const
{
  if ( !this->hasIndex( row, column, parent ) ) return QModelIndex();
  
  ProvenanceTreeItem* parent_item;
  if ( !parent.isValid() )
  {
    parent_item = this->private_->root_item_.get();
  }
  else
  {
    parent_item = static_cast< ProvenanceTreeItem* >( parent.internalPointer() );
  }

  if ( row < parent_item->children_.size() )
  {
    return this->createIndex( row, column, parent_item->children_[ row ].get() );
  }
  
  return QModelIndex();
}

QModelIndex ProvenanceTreeModel::parent( const QModelIndex& index ) const
{
  if ( !index.isValid() ) return QModelIndex();

  ProvenanceTreeItem* child_item = static_cast< ProvenanceTreeItem* >( index.internalPointer() );
  ProvenanceTreeItem* parent_item = child_item->parent_;
  if ( parent_item == this->private_->root_item_.get() )
  {
    return QModelIndex();
  }
    
  return this->createIndex( parent_item->row_, 0, parent_item );
}

int ProvenanceTreeModel::rowCount( const QModelIndex& parent ) const
{
  if ( parent.column() > 0 ) return 0;

  ProvenanceTreeItem* parent_item;
  if ( !parent.isValid() )
  {
    parent_item = this->private_->root_item_.get();
  }
  else
  {
    parent_item = static_cast< ProvenanceTreeItem* >( parent.internalPointer() );
  }

  return static_cast< int >( parent_item->children_.size() );
}

QVariant ProvenanceTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    return this->private_->root_item_->data( section );
  }
  
  return QVariant();
}

} // end namespace Seg3D
