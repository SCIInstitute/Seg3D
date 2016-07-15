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

#include <gtest/gtest.h>

#include <Core/Action/Action.h>

#include <string>

using namespace Core;

static std::string dummyActionDef("<?xml version=\"1.0\"?><action name=\"DummyAction\">dummy action</action>");

class DummyAction : public Action
{
public:
  virtual ActionInfoHandle get_action_info() const override
  {
    return ActionInfoHandle( new ActionInfo(dummyActionDef) );
  }
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override { return true; }
  virtual bool validate( ActionContextHandle& context ) override { return true; }
};

TEST(ActionTests, EmptyAction)
{
  DummyAction action;
  ActionContextHandle context( new ActionContext() );
  ActionResultHandle result( new ActionResult() );

  std::string def = action.get_definition();
  ASSERT_EQ( def, dummyActionDef.append("\n") );

  std::string type = action.get_type();
  ASSERT_EQ( type, "DummyAction" );

  std::string usage = action.get_usage();
  ASSERT_EQ( usage, "DummyAction" );

  std::string actionString = action.export_to_string();
  ASSERT_EQ( actionString, "DummyAction " );

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}