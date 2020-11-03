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

#ifndef APPLICATION_DATABASEMANAGER_DATABASEMANAGER_H
#define APPLICATION_DATABASEMANAGER_DATABASEMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <map>

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>

namespace Seg3D
{

typedef std::vector< std::map< std::string, boost::any > > ResultSet;

// Forward declaration
class DatabaseManager;
typedef boost::shared_ptr< DatabaseManager > DatabaseManagerHandle;
class DatabaseManagerPrivate;

// Class definition
class DatabaseManager : public boost::noncopyable
{
  // -- Constructor/Destructor --
public:
  // Default constructor
  DatabaseManager();

  // Copy constructor
  DatabaseManager( const DatabaseManager& src );

  virtual ~DatabaseManager();

public:
  /// RUN_SQL_STATEMENT:
  /// Execute the given SQL statement on the database. If the statement generates
  /// any results, they will be put in the result set.
  /// Returns true on success, otherwise false.
  bool run_sql_statement( const std::string& sql_str, ResultSet& results, std::string& error );

  /// RUN_SQL_STATEMENT:
  /// Execute the given SQL statement on the database.
  /// Returns true on success, otherwise false.
  bool run_sql_statement( const std::string& sql_str, std::string& error );

  /// RUN_SQL_SCRIPT:
  /// Execute multiple SQL statements sequentially.
  bool run_sql_script( const std::string& sql_str, std::string& error );

  /// SAVE_DATABASE:
  /// Save the database to disk
  bool save_database( const boost::filesystem::path& database_file, std::string& error );

  /// LOAD_DATABASE:
  /// Load the database from disk
  bool load_database( const boost::filesystem::path& database_file, std::string& error );

  /// GET_LAST_INSERT_ROWID:
  /// Return the row ID of last successful insert statement.
  long long get_last_insert_rowid();

  /// GET_COLUMN_METADATA:
  /// Get metadata about a specific column of a specific database table.
  /// Returns true if the table and column exist, otherwise false.
  bool get_column_metadata( const std::string& table_name, const std::string& column_name,
    char const** data_type = NULL, char const** coll_seq = NULL,
    int* not_null = NULL, int* primary_key = NULL, int* auto_inc = NULL );

private:
  boost::shared_ptr< DatabaseManagerPrivate > private_;

public:
  /// ESCAPEQUOTES:
  /// Escape the quotes(') in the string so it can be used as text in a SQL statement.
  static std::string EscapeQuotes( const std::string& str );
};

} // end namespace seg3d

#endif
