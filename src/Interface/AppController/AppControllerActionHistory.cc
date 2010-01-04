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

#include <Interface/AppController/AppControllerActionHistory.h>

namespace Seg3D {

AppControllerActionHistory::AppControllerActionHistory(QObject* parent) :
  QAbstractTableModel(parent),
  history_(ActionHistory::Instance())
{
}

AppControllerActionHistory::~AppControllerActionHistory()
{
}

int
AppControllerActionHistory::rowCount(const QModelIndex& /*index*/) const
{
  return (static_cast<int>(history_->history_size()));
}

int
AppControllerActionHistory::columnCount(const QModelIndex& /*index*/) const
{
  return (2);
}

QVariant 
AppControllerActionHistory::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return QVariant();
  
  if (role == Qt::TextAlignmentRole)
  {
    return int(Qt::AlignLeft|Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole)
  {
    int sz = static_cast<int>(history_->history_size());
    if (index.row() < sz)
    {
      if (index.column() == 0)
      {
        ActionHandle action = history_->action(sz-index.row()-1);
        if (action.get() == 0) return QString("");
        return QString::fromStdString(action->export_to_string());
      }
      else
      {
        ActionResultHandle result = history_->result(sz-index.row()-1);
        if (result.get() == 0) return QString("");
        return QString::fromStdString(result->export_to_string());      
      }
    }
    else
    {
      return QVariant();
    }
  }
  else if (role == Qt::SizeHintRole)
  {
    if (index.column() == 0) return QSize(200,12);
    else return QSize(100,12);
  }
  else
  {
    return QVariant();
  }
}

QVariant 
AppControllerActionHistory::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole || orientation == Qt::Vertical)
  {
    return QVariant();
  }
  
  if (section == 0) return QString("Action");
  if (section == 1) return QString("Result");
  else return QVariant();
}

void 
AppControllerActionHistory::updateHistory() 
{ 
  reset(); 
}

} // end namespace Seg3D
