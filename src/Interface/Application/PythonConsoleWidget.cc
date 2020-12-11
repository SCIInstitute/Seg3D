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

// Python
// NOTE: This one has to be here: Because the Python source code is sub standard, we need to include
// this first, to avoid trouble with the macros that Python defines.
#ifdef BUILD_WITH_PYTHON_LEGACY
#include <Python.h>
#endif

// QT includes
#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QPointer>
#include <QTextCursor>
#include <QTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QtCore/QMimeData>

// Core includes
#include <Core/Interface/Interface.h>

// QtUtils includes
#include <QtUtils/Utils/QtPointer.h>

// Application includes
#include <Core/Python/PythonInterpreter.h>

// Interface includes
#include <Interface/Application/PythonConsoleWidget.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class PythonConsoleEdit
//////////////////////////////////////////////////////////////////////////

class PythonConsoleEdit;
typedef QPointer< PythonConsoleEdit > PythonConsoleEditQWeakPointer;

class PythonConsoleEdit : public QTextEdit
{
public:
  PythonConsoleEdit( PythonConsoleWidget* parent );

  virtual void keyPressEvent( QKeyEvent* e ) override;
  //virtual void focusOutEvent( QFocusEvent* e );

  virtual void insertFromMimeData( const QMimeData *source ) override;

  const int document_end();
  QString& command_buffer();
  void update_command_buffer();
  void replace_command_buffer( const QString& text );
  void issue_command();

  void prompt( const std::string text );
  void print_output( const std::string text );
  void print_error( const std::string text );
  void print_command( const std::string text );

public:
  // The beginning of the area of interactive input, outside which
  // changes can't be made to the text edit contents.
  int interactive_position_;
  // Command history plus the current command buffer
  QStringList command_history_;
  // Current position in the command history
  int command_position_;

public:
  static void Prompt( PythonConsoleEditQWeakPointer edit, const std::string text );
  static void PrintOutput( PythonConsoleEditQWeakPointer edit, const std::string text );
  static void PrintError( PythonConsoleEditQWeakPointer edit, const std::string text );
  static void PrintCommand( PythonConsoleEditQWeakPointer edit, const std::string text );
};

PythonConsoleEdit::PythonConsoleEdit( PythonConsoleWidget* parent ) :
  QTextEdit( parent )
{
  this->interactive_position_ = this->document_end();
  this->setTabChangesFocus( false );
  this->setAcceptDrops( false );
  this->setAcceptRichText( false );
  this->setUndoRedoEnabled( false );
  //this->setLineWrapMode( QTextEdit::NoWrap );
  this->document()->setMaximumBlockCount( 2048 );

  QFont f;
  f.setFamily( "Courier" );
  f.setStyleHint( QFont::TypeWriter );
  f.setFixedPitch( true );

  // Set the tab width to 4 spaces
  QFontMetrics fm( f, this );
  this->setTabStopWidth( fm.width( "    " ) );

  QTextCharFormat format;
  format.setFont( f );
  format.setForeground( QColor( 0, 0, 0 ) );
  this->setCurrentCharFormat( format );

  this->command_history_.append("");
  this->command_position_ = 0;
}

void PythonConsoleEdit::keyPressEvent( QKeyEvent* e )
{
  this->setTextColor( Qt::black );
  QTextCursor text_cursor = this->textCursor();

  // Whether there's a current selection
  const bool selection = text_cursor.anchor() != text_cursor.position();
  // Whether the cursor overlaps the history area
  const bool history_area = text_cursor.anchor() < this->interactive_position_ ||
    text_cursor.position() < this->interactive_position_;

  // Allow copying anywhere in the console ...
  if ( e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier )
  {
    if ( selection )
    {
      this->copy();
    }
    else
    {
      Core::PythonInterpreter::Instance()->interrupt();
    }
    e->accept();
    return;
  }

  // Allow cut only if the selection is limited to the interactive area ...
  if ( e->key() == Qt::Key_X && e->modifiers() == Qt::ControlModifier )
  {
    if( selection && !history_area )
    {
      this->cut();
    }
    e->accept();
    return;
  }

  // Allow paste only if the selection is in the interactive area ...
  if ( e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier )
  {
    if ( ! history_area )
    {
      const QMimeData* const clipboard = QApplication::clipboard()->mimeData();
      const QString text = clipboard->text();
      if( ! text.isNull() )
      {
        text_cursor.insertText( text );
        this->update_command_buffer();
      }
    }

    e->accept();
    return;
  }

  // Force the cursor back to the interactive area
  if ( history_area && e->key() != Qt::Key_Control )
  {
    text_cursor.setPosition( this->document_end() );
    this->setTextCursor( text_cursor );
  }

  switch( e->key() )
  {
  case Qt::Key_Up:
    e->accept();
    if ( this->command_position_ > 0 )
    {
      this->replace_command_buffer( this->command_history_[ --this->command_position_ ] );
    }
    break;

  case Qt::Key_Down:
    e->accept();
    if ( this->command_position_ < this->command_history_.size() - 2 )
    {
      this->replace_command_buffer( this->command_history_[ ++this->command_position_ ] );
    }
    else
    {
      this->command_position_ = this->command_history_.size() - 1;
      this->replace_command_buffer( "" );
    }
    break;

  case Qt::Key_Left:
    if ( text_cursor.position() > this->interactive_position_ )
    {
      QTextEdit::keyPressEvent( e );
    }
    else
    {
      e->accept();
    }
    break;

  case Qt::Key_Delete:
    e->accept();
    QTextEdit::keyPressEvent( e );
    this->update_command_buffer();
    break;

  case Qt::Key_Backspace:
    e->accept();
    if ( text_cursor.position() > this->interactive_position_ )
    {
      QTextEdit::keyPressEvent( e );
      this->update_command_buffer();
    }
    break;

  case Qt::Key_Tab:
    e->accept();
    QTextEdit::keyPressEvent( e );
    break;

  case Qt::Key_Home:
    e->accept();
    text_cursor.setPosition( this->interactive_position_ );
    this->setTextCursor( text_cursor );
    break;

  case Qt::Key_Return:
  case Qt::Key_Enter:
    e->accept();
    text_cursor.setPosition( this->document_end() );
    this->setTextCursor( text_cursor );
    this->issue_command();
    break;

  default:
    e->accept();
    QTextEdit::keyPressEvent( e );
    this->update_command_buffer();
    break;
  }
}

void PythonConsoleEdit::insertFromMimeData( const QMimeData *source )
{
  // see also PythonConsoleEdit::keyPressEvent
  this->setTextColor( Qt::black );
  QTextCursor text_cursor = this->textCursor();
  const bool history_area = text_cursor.anchor() < this->interactive_position_ ||
  text_cursor.position() < this->interactive_position_;

  if ( ! history_area )
  {
    const QString text = source->text();
    if( ! text.isNull() )
    {
      text_cursor.insertText( text );
      this->update_command_buffer();
    }
  }
}

const int PythonConsoleEdit::document_end()
{
  QTextCursor c( this->document() );
  c.movePosition( QTextCursor::End );
  return c.position();
}

QString& PythonConsoleEdit::command_buffer()
{
  return this->command_history_.back();
}

void PythonConsoleEdit::update_command_buffer()
{
  this->command_buffer() = this->toPlainText().mid( this->interactive_position_ );
}

void PythonConsoleEdit::replace_command_buffer( const QString& text )
{
  this->command_buffer() = text;

  QTextCursor c( this->document() );
  c.setPosition( this->interactive_position_ );
  c.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
  c.removeSelectedText();

  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground( Qt::black );
  c.setCharFormat( char_format );
  c.insertText( text );
}

void PythonConsoleEdit::issue_command()
{
  QString command = this->command_buffer();
  // Update the command history.
  if ( ! command.isEmpty() )
  {
    this->command_history_.push_back( "" );
    this->command_position_ = this->command_history_.size() - 1;
  }

  QTextCursor c( this->document() );
  c.movePosition( QTextCursor::End );
  c.insertText( "\n" );

  this->interactive_position_ = this->document_end();

  auto lines = command.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
  for (const auto& line : lines)
  {
    if ( ! line.isEmpty() )
      Core::PythonInterpreter::Instance()->run_string( line.toStdString() );
  }
}

void PythonConsoleEdit::prompt( const std::string text )
{
  QString text_to_insert = QString::fromStdString( text );
  // remove whitespace from beginning and end
  text_to_insert = text_to_insert.trimmed();

  QTextCursor text_cursor( this->document() );
  // Move the cursor to the end of the document
  text_cursor.setPosition( this->document_end() );

  // if the cursor is currently on a clean line, do nothing, otherwise we move
  // the cursor to a new line before showing the prompt.
  text_cursor.movePosition( QTextCursor::StartOfLine );
  int startpos = text_cursor.position();
  text_cursor.movePosition( QTextCursor::EndOfLine );
  int endpos = text_cursor.position();

  // Make sure the new text will be in the right color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground( Qt::black );
  text_cursor.setCharFormat( char_format );

  if ( endpos != startpos )
  {
    text_cursor.insertText( "\n" );
  }

  text_cursor.insertText( text_to_insert );
  this->setTextCursor( text_cursor );
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_output( const std::string text )
{
  QTextCursor text_cursor( this->document() );
  // Move the cursor to the end of the document
  text_cursor.setPosition( this->document_end() );

  // Set the proper text color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground( Qt::black );
  text_cursor.setCharFormat( char_format );

  text_cursor.insertText( QString::fromStdString( text ) );
  this->setTextCursor( text_cursor );
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_error( const std::string text )
{
  QTextCursor text_cursor( this->document() );
  // Move the cursor to the end of the document
  text_cursor.setPosition( this->document_end() );

  // Set the proper text color
  QTextCharFormat char_format = this->currentCharFormat();
  char_format.setForeground( Qt::red );
  text_cursor.setCharFormat( char_format );

  text_cursor.insertText( QString::fromStdString( text ) );
  this->setTextCursor( text_cursor );
  this->interactive_position_ = this->document_end();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::print_command( const std::string text )
{
  QTextCursor text_cursor = this->textCursor();
  text_cursor.setPosition( this->document_end() );
  this->setTextCursor( text_cursor );
  text_cursor.insertText( QString::fromStdString( text ) );
  this->update_command_buffer();
  this->ensureCursorVisible();
}

void PythonConsoleEdit::Prompt( PythonConsoleEditQWeakPointer edit, const std::string text )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( edit,
    boost::bind( &PythonConsoleEdit::prompt, edit.data(), text ) ) );
}

void PythonConsoleEdit::PrintOutput( PythonConsoleEditQWeakPointer edit, const std::string text )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( edit,
    boost::bind( &PythonConsoleEdit::print_output, edit.data(), text ) ) );
}

void PythonConsoleEdit::PrintError( PythonConsoleEditQWeakPointer edit, const std::string text )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( edit,
    boost::bind( &PythonConsoleEdit::print_error, edit.data(), text ) ) );
}

void PythonConsoleEdit::PrintCommand( PythonConsoleEditQWeakPointer edit, const std::string text )
{
  Core::Interface::PostEvent( QtUtils::CheckQtPointer( edit,
    boost::bind( &PythonConsoleEdit::print_command, edit.data(), text ) ) );
}

//////////////////////////////////////////////////////////////////////////
// Class PythonConsoleWidgetPrivate
//////////////////////////////////////////////////////////////////////////

class PythonConsoleWidgetPrivate
{
public:
  PythonConsoleEdit* console_edit_;
};

//////////////////////////////////////////////////////////////////////////
// Class PythonConsoleWidget
//////////////////////////////////////////////////////////////////////////

PythonConsoleWidget::PythonConsoleWidget( QWidget* parent ) :
  QtUtils::QtCustomDialog( parent ),
  private_( new PythonConsoleWidgetPrivate )
{
  this->private_->console_edit_ = new PythonConsoleEdit( this );
  QVBoxLayout* const layout = new QVBoxLayout( this );
  layout->setMargin( 0 );
  layout->addWidget( this->private_->console_edit_ );

  this->setMinimumSize( 500, 500 );

  setWindowTitle("Python console");

  PythonConsoleEditQWeakPointer qpointer( this->private_->console_edit_ );

  this->add_connection( Core::PythonInterpreter::Instance()->prompt_signal_.connect(
    boost::bind( &PythonConsoleEdit::Prompt, qpointer, _1 ) ) );
  this->add_connection( Core::PythonInterpreter::Instance()->output_signal_.connect(
    boost::bind( &PythonConsoleEdit::PrintOutput, qpointer, _1 ) ) );
  this->add_connection( Core::PythonInterpreter::Instance()->error_signal_.connect(
    boost::bind( &PythonConsoleEdit::PrintError, qpointer, _1 ) ) );

  Core::PythonInterpreter::Instance()->print_banner();
}

PythonConsoleWidget::~PythonConsoleWidget()
{
  this->disconnect_all();
}

} // end namespace Seg3D
