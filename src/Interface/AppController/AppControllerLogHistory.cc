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

#include <Interface/AppController/AppControllerLogHistory.h>

namespace Seg3D {

AppControllerLogHistory::AppControllerLogHistory(QObject* parent) :
  QAbstractTableModel(parent),
  history_(Utils::LogHistory::Instance())
{
}

AppControllerLogHistory::~AppControllerLogHistory()
{
}

int
AppControllerLogHistory::rowCount(const QModelIndex& /*index*/) const
{
  return (static_cast<int>(history_->history_size()));
}

int
AppControllerLogHistory::columnCount(const QModelIndex& /*index*/) const
{
  return (1);
}

QVariant 
AppControllerLogHistory::data(const QModelIndex& index, int role) const
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
      Utils::LogHistory::log_entry_type log_entry = history_->log_entry(sz-index.row()-1);
      if (index.column() == 0)
      {
        return (QString::fromStdString(log_entry.second));
      }
    }
    else
    {
      return QVariant();
    }
  }
  else
  {
    return QVariant();
  }
  
  return QVariant();
}

QVariant 
AppControllerLogHistory::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole || orientation == Qt::Vertical)
  {
    return QVariant();
  }
  
  if (section == 0) return QString("Log entry");
  else return QVariant();
}

void 
AppControllerLogHistory::updateHistory() 
{ 
  reset(); 
}

} // end namespace Seg3D
