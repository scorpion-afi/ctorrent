/*
 * regular_file.h
 *
 *  Created on: Jun 16, 2018
 *      Author: sergs
 */

#ifndef REGULAR_FILE_H
#define REGULAR_FILE_H

#include <string>
#include <ios>

#include "object.h"

/* this class manages a file, not an open file like an std::fstream one;
 * a file gets removed (by std::remove) if an object gets destroyed;
 * no copy semantic;
 * thread-safe */
class regular_file : public object
{
public:
  /* create or [wrap/adopt an already created] file with @c file_name name;
   * adopt - specify if a file is already created and this object just has to wrap it;
   *
   * Note that a file gets created with an std::ios::out/trunc combination of flags (by default),
   * so if the file exists its content will be truncated */
  regular_file( const std::string& file_name, bool adopt = false,
                std::ios::openmode mode = std::ios::out | std::ios::trunc );
  ~regular_file();

  regular_file( const regular_file& that ) = delete;
  regular_file& operator=( const regular_file& that ) = delete;

  regular_file( regular_file&& that ) = default;
  regular_file& operator=( regular_file&& that ) = default;


  const std::string& get_file_name() const { return file_name; }

  /* TODO: is it good to have such a functionality for the class that
   *       manages a NON-open file? */
  /* temporary open a file for writing (std::ios::app), write (to the end) and then close the file */
  regular_file& operator<<( const std::string& str );

private:
  const std::string file_name;
};

#endif /* REGULAR_FILE_H */
