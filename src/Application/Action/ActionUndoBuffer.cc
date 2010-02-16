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

#include <Application/Action/ActionUndoBuffer.h>
#include <Application/Interface/Interface.h>


namespace Seg3D {

ActionUndoBuffer::ActionUndoBuffer() :
  undo_buffer_size_(500)
{
}

ActionUndoBuffer::~ActionUndoBuffer()
{
}

void
ActionUndoBuffer::add_undo_action(std::string& tag,
                           ActionHandleList& undo_actions,
                           ActionHandle& redo_action)
{
  // Filter out undo/redo actions that are not valid
  if (redo_action.get() == 0)
  {
    SCI_LOG_ERROR("Registration of a redo action that is empty");
    return;  
  }
  
  for (size_t j=0; j<undo_actions.size(); j++)
  {
    if (undo_actions[j].get() == 0)
    {
      SCI_LOG_ERROR("Registration of an undo action that is empty");
      return;
    }

  }

  if (tag == "")
  {
    SCI_LOG_ERROR("Undo actions need to be tagged with a label");
    return;
  }

  ActionUndoItemHandle item = ActionUndoItemHandle(
    new ActionUndoItem(tag,undo_actions,redo_action));  

  // Lock the lists
  boost::unique_lock<boost::mutex> lock(buffer_mutex_);    

  // Add this item to the front of this list
  backward_buffer_.push_front(item);
  
  // If there are too many items in the list pop the last ones
  while (backward_buffer_.size() > undo_buffer_size_) backward_buffer_.pop_back();
  
  // Clear the redo buffer 
  forward_buffer_.clear();  
}    

void
ActionUndoBuffer::tags_changed()
{
  std::string undo_tag;
  std::string redo_tag;
  if (!(backward_buffer_.empty())) undo_tag = backward_buffer_.front()->tag();
  if (!(forward_buffer_.empty())) redo_tag = forward_buffer_.front()->tag();

  tags_changed_(undo_tag,redo_tag);
}

void 
ActionUndoBuffer::set_undo_buffer_size(size_t size)
{
  // Lock the lists
  boost::unique_lock<boost::mutex> lock(buffer_mutex_);    
  undo_buffer_size_ = size;

  // If there are too many items in the list pop the last ones
  while (backward_buffer_.size() > undo_buffer_size_) backward_buffer_.pop_back();    
}


bool
ActionUndoBuffer::query_current_undo_action(ActionUndoItemHandle undo_action)
{
  if (backward_buffer_.empty()) return (false);
  undo_action = backward_buffer_.front();
  return (true);
}

void
ActionUndoBuffer::undo_action(ActionContextHandle& context)
{
  // The undo item to play back
  ActionUndoItemHandle item;
  
  {
    // Lock the lists
    boost::unique_lock<boost::mutex> lock(buffer_mutex_);    

    // Check whether there is an action to undo
    if (!(backward_buffer_.empty()))
    {
      // Update the state by moving the first undo action back to the 
      // front of the redo list
      ActionUndoItemHandle item = backward_buffer_.front();
      backward_buffer_.pop_front();
      // This action is schedule for redo
      forward_buffer_.push_front(item);
    }

    tags_changed();
  }

  // Undo the last action
  for(size_t j=0; j<item->undo_actions().size(); j++)
  {
    run_action(item->undo_actions()[j],context);
  }
}


void
ActionUndoBuffer::redo_action(ActionContextHandle& context)
{
  // The item to redo
  ActionUndoItemHandle item;
  
  {
    // Lock the lists
    boost::unique_lock<boost::mutex> lock(buffer_mutex_);    

    // Check whether there is an action to undo
    if (!(forward_buffer_.empty()))
    {
      // Update the state by moving the first undo action back to the 
      // front of the redo list
      ActionUndoItemHandle item = forward_buffer_.front();
      forward_buffer_.pop_front();
      // This action is schedule for redo
      backward_buffer_.push_front(item);
    }

    tags_changed();
  }

  // Redo the last action
  run_action(item->redo_action(),context);
}


bool
ActionUndoBuffer::has_undo_action()
{
  boost::unique_lock<boost::mutex> lock(buffer_mutex_);    
  return (!(backward_buffer_.empty()));
}


bool
ActionUndoBuffer::has_redo_action()
{
  boost::unique_lock<boost::mutex> lock(buffer_mutex_);    
  return (!(forward_buffer_.empty()));
}

void
ActionUndoBuffer::run_action(ActionHandle action, 
                             ActionContextHandle action_context)
{

  SCI_LOG_DEBUG(std::string("Processing Undo/Redo Action: ")+action->type());  

  // Step (1): An action needs to be validated before it can be executed. 
  // The validation is a separate step as invalid actions should nor be 
  // posted to the observers recording what the program does

  SCI_LOG_DEBUG("Validating Undo/Redo Action");  
  if (!(action->validate(action_context)))
  {
    // The action  context should return unavailable or invalid
    if (action_context->status() != ACTION_UNAVAILABLE_E)
    {
      action_context->report_status(ACTION_INVALID_E);
    }
    action_context->report_done();
    return;
  }

  // NOTE: NO OBSERVERS - they observe the undo or redo action.
  // The observers do not see these action, as undo action maybe actually
  // multiple actions.
  
  // Step (2): Run action from the context that was provided. And if the action
  // was synchronous a done signal is triggered in the context, to inform the
  // program whether the action succeeded.
  SCI_LOG_DEBUG("Running Undo/Redo Action");    

  ActionResultHandle result;
  if(!(action->run(action_context, result)))
  {
    action_context->report_status(ACTION_ERROR_E);
    action_context->report_done();
  }
  
  return;
}



void
AddUndoAction(std::string undo_tag, ActionHandle undo, ActionHandle redo)
{
  ActionHandleList undo_list(1);
  undo_list[0] = undo;
  ActionUndoBuffer::Instance()->add_undo_action(undo_tag,undo_list,redo);
}

void
AddUndoAction(std::string undo_tag, ActionHandle undo1, 
              ActionHandle undo2, ActionHandle redo)
{
  ActionHandleList undo_list(2);
  undo_list[0] = undo1;
  undo_list[1] = undo2;
  ActionUndoBuffer::Instance()->add_undo_action(undo_tag,undo_list,redo);
}

void
AddUndoAction(std::string undo_tag, ActionHandle undo1, 
              ActionHandle undo2, ActionHandle undo3,
              ActionHandle redo)
{
  ActionHandleList undo_list(3);
  undo_list[0] = undo1;
  undo_list[1] = undo2;
  undo_list[2] = undo3;
  ActionUndoBuffer::Instance()->add_undo_action(undo_tag,undo_list,redo);
}


void
AddUndoAction(std::string undo_tag, ActionHandleList& undo_list, 
              ActionHandle redo)
{
  ActionUndoBuffer::Instance()->add_undo_action(undo_tag,undo_list,redo);
}
// Singleton interface needs to be defined somewhere
Utils::Singleton<ActionUndoBuffer> ActionUndoBuffer::instance_;

} // end namespace Seg3D
