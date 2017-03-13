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
using namespace ::testing;

static std::string dummyActionDef("<?xml version=\"1.0\"?><action name=\"DummyAction\">dummy action</action>");
static std::string dummyActionDefWithParam("<?xml version=\"1.0\"?><action name=\"DummyAction\">dummy action</action><key name=\"param_required\">param required</key>");
static std::string dummyActionDefWithParams("<?xml version=\"1.0\"?><action name=\"DummyAction\">dummy action</action><key name=\"param_required\">param required</key><key name=\"param_optional\" default=\"optional\">param optional</key>");
static std::string dummyActionDefWithParamsAndProperties("<?xml version=\"1.0\"?><action name=\"DummyAction\">dummy action</action><key name=\"param_required\">param required</key><key name=\"param_optional\" default=\"optional\">param optional</key><key name=\"sandbox\" default=\"-1\">sandbox</key><property name=\"sandbox\">nonpersistent</property><property>changes_project_data</property><property>is_undoable</property>");

class DummyAction : public Action
{
public:
  virtual ActionInfoHandle get_action_info() const override
  {
    return ActionInfoHandle( new ActionInfo( dummyActionDef ) );
  }
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override { return true; }
  virtual bool validate( ActionContextHandle& context ) override { return true; }
};

class DummyActionWithParam : public Action
{
public:
  DummyActionWithParam()
  {
    this->add_parameter( this->param_required_ );
  }
  virtual ActionInfoHandle get_action_info() const override
  {
    return ActionInfoHandle( new ActionInfo( dummyActionDefWithParam ) );
  }
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override { return true; }
  virtual bool validate( ActionContextHandle& context ) override { return true; }
private:
  std::string param_required_;
};

class DummyActionWithParams : public Action
{
public:
  DummyActionWithParams()
  {
    this->add_parameter( this->param_required_ );
    this->add_parameter( this->param_optional_ );
  }
  virtual ActionInfoHandle get_action_info() const override
  {
    return ActionInfoHandle( new ActionInfo( dummyActionDefWithParams ) );
  }
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override { return true; }
  virtual bool validate( ActionContextHandle& context ) override { return true; }
private:
  std::string param_required_;
  std::string param_optional_;
};

class DummyActionWithParamsAndProperties : public Action
{
public:
  DummyActionWithParamsAndProperties()
  {
    this->add_parameter( this->param_required_ );
    this->add_parameter( this->param_optional_ );
  }
  virtual ActionInfoHandle get_action_info() const override
  {
    return ActionInfoHandle( new ActionInfo( dummyActionDefWithParamsAndProperties ) );
  }
  virtual bool run( ActionContextHandle& context, ActionResultHandle& result ) override { return true; }
  virtual bool validate( ActionContextHandle& context ) override { return true; }
private:
  std::string param_required_;
  std::string param_optional_;
};


class ActionTests : public Test
{
protected:
  virtual void SetUp()
  {
    context.reset( new ActionContext() );
    result.reset( new ActionResult() );
    actionImportString = "dummyaction";
    actionString = "DummyAction ";
    actionTypeUsageString = "DummyAction";
  }

  ActionContextHandle context;
  ActionResultHandle result;
  std::string actionImportString;
  std::string actionString;
  std::string actionTypeUsageString;
};

TEST_F(ActionTests, EmptyAction)
{
  DummyAction action;

  std::string def = action.get_definition();
  ASSERT_EQ( def, dummyActionDef.append("\n") );

  std::string type = action.get_type();
  ASSERT_EQ( type, actionTypeUsageString );

  std::string usage = action.get_usage();
  ASSERT_EQ( usage, actionTypeUsageString );

  std::string actionExportString = action.export_to_string();
  ASSERT_EQ( actionExportString, actionString );

  std::string pythonActionExportString = action.export_to_python_string();
  ASSERT_EQ( pythonActionExportString, "dummyaction();");

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromString)
{
  DummyAction action;
  action.import_from_string( actionImportString );

  std::string actionExportString = action.export_to_string();
  ASSERT_EQ( actionExportString, actionString );

  std::string pythonActionExportString = action.export_to_python_string();
  ASSERT_EQ( pythonActionExportString, "dummyaction();");

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromStringWithError)
{
  DummyAction action;
  std::string error;
  action.import_from_string( actionImportString, error );

  std::string actionExportString = action.export_to_string();
  ASSERT_EQ( actionExportString, actionString );

  std::string pythonActionExportString = action.export_to_python_string();
  ASSERT_EQ( pythonActionExportString, "dummyaction();");

  ASSERT_TRUE( error.empty() );

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromStringWithParam)
{
  DummyActionWithParam action;
  std::string error;

  std::string actionImportStringWithParam( "dummyaction param_required=param" );
  bool retVal = false;
  ASSERT_NO_THROW( retVal = action.import_from_string( actionImportStringWithParam, error ) );
  ASSERT_TRUE( retVal );
  std::string actionExportString = action.export_to_string();
  ASSERT_EQ( actionExportString, "DummyAction param_required=\'param\' " );

  std::string pythonActionExportString = action.export_to_python_string();
  ASSERT_EQ( pythonActionExportString, "dummyaction(param_required='param');");

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromStringWithParamError)
{
  DummyActionWithParam action;
  std::string error;

  std::string actionImportStringWithParam( "dummyaction" );
  bool retVal = false;
  ASSERT_NO_THROW( retVal = action.import_from_string( actionImportStringWithParam, error ) );
  ASSERT_FALSE( retVal );

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromStringWithParamsError)
{
  DummyActionWithParams action;
  std::string error;

  std::string actionImportStringWithParams( "dummyaction" );
  bool retVal = false;
  ASSERT_NO_THROW( retVal = action.import_from_string( actionImportStringWithParams, error ) );
  ASSERT_FALSE( retVal );

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

TEST_F(ActionTests, ActionImportFromStringWithParamsWithOptionalError)
{
  DummyActionWithParams action;
  std::string error;

  std::string actionImportStringWithParams( "dummyaction param_optional=param" );
  bool retVal = false;
  ASSERT_NO_THROW( retVal = action.import_from_string( actionImportStringWithParams, error ) );
  ASSERT_FALSE( retVal );

  ASSERT_TRUE( action.translate( context ) );
  ASSERT_TRUE( action.post_create( context ) );
  ASSERT_TRUE( action.validate( context ) );
  ASSERT_TRUE( action.run( context, result ) );
  ASSERT_FALSE( action.is_undoable() );
  ASSERT_FALSE( action.changes_project_data() );
}

