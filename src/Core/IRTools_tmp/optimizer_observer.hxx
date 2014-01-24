// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: t -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// File         : optimizer_observer.hxx
// Author       : Pavel A. Koshevoy
// Created      : Fri Mar 23 12:47:03 MDT 2007
// Copyright    : (C) 2004-2008 University of Utah
// License      : GPLv2
// Description  : An optimizer observer class.

#ifndef OPTIMIZER_OBSERVER_HXX_
#define OPTIMIZER_OBSERVER_HXX_

// TODO: temporary
#include <Core/IRTools_tmp/the_log.hxx>

// ITK includes:
#include <itkCommand.h>


//----------------------------------------------------------------
// optimizer_observer_t
//
template <typename TOptimizer>
class optimizer_observer_t : public itk::Command
{
public:
  typedef optimizer_observer_t Self;
  typedef itk::Command Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  
  itkNewMacro(Self);
  
  void Execute(itk::Object *caller, const itk::EventObject & event)
  { Execute((const itk::Object *)(caller), event); }
  
  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    if (typeid(event) != typeid(itk::IterationEvent)) return;
    
    const TOptimizer * optimizer = dynamic_cast<const TOptimizer *>(object);
    *log_ << (unsigned int)(optimizer->GetCurrentIteration()) << '\t'
	  << optimizer->GetValue() << '\t'
	  << optimizer->GetCurrentPosition() << endl;
  }
  
  the_log_t * log_;
  
protected:
  optimizer_observer_t():
    log_(cerr_log())
  {}
};


#endif // OPTIMIZER_OBSERVER_HXX_
