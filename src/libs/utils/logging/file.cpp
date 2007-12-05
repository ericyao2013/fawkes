
/***************************************************************************
 *  file.cpp - Fawkes file logger
 *
 *  Created: Tue Jan 16 16:56:49 2007
 *  Copyright  2006-2007  Tim Niemueller [www.niemueller.de]
 *             2007       Daniel Beck
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#include <utils/logging/file.h>
#include <utils/system/file.h>

#include <core/threading/mutex.h>

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <cstdio>

/** @class FileLogger logging/file.h
 * Interface for logging to a specified file.
 * The FileLogger will pipe all output into the given file. The
 * output will be prepended by a single character which determines the 
 * type of output (E for error, W for warning, etc.).
 *
 */

/** Constructor. 
 * @param filename the name of the log-file
 * @param log_level minimum log level
 */
FileLogger::FileLogger(const char* filename, LogLevel log_level)
  : Logger(log_level)
{
  try {
    log_file = new File(filename, File::ADD_SUFFIX);
  } catch (UnableToOpenFileException& e) {
    throw;
  }

  now = (struct timeval *)malloc(sizeof(struct timeval));
  now_s = (struct tm *)malloc(sizeof(struct tm));

  mutex = new Mutex();
}


/** Destructor. */
FileLogger::~FileLogger()
{
  free(now);
  free(now_s);
  
  delete log_file;
  delete mutex;
}


/** Log debug message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 * @param va variadic argument list
 */
void
FileLogger::vlog_debug(const char* component, const char* format, va_list va)
{
  if (log_level <= LL_DEBUG ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s: ", "D", now_s->tm_hour,
	    now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
    vfprintf(log_file->stream(), format, va);
    fprintf(log_file->stream(), "\n");
    mutex->unlock();
  }
}


/** Log informational message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 * @param va variadic argument list
 */
void
FileLogger::vlog_info(const char *component, const char *format, va_list va)
{
  if (log_level <= LL_INFO ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s: ", "I", now_s->tm_hour,
	    now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
    vfprintf(log_file->stream(), format, va);
    fprintf(log_file->stream(), "\n");
    mutex->unlock();
  }
}


/** Log warning message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 * @param va variadic argument list
 */
void
FileLogger::vlog_warn(const char *component, const char *format, va_list va)
{
  if (log_level <= LL_WARN ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s: ", "W", now_s->tm_hour,
	    now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
    vfprintf(log_file->stream(), format, va);
    fprintf(log_file->stream(), "\n");
    mutex->unlock();
  }
}


/** Log error message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 * @param va variadic argument list
 */
void
FileLogger::vlog_error(const char *component, const char *format, va_list va)
{
  if (log_level <= LL_ERROR ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s: ", "E", now_s->tm_hour,
	    now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
    vfprintf(log_file->stream(), format, va);
    fprintf(log_file->stream(), "\n");
    mutex->unlock();
  }
}


/** Log debug message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 */
void
FileLogger::log_debug(const char *component, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  vlog_debug(component, format, arg);
  va_end(arg);
}


/** Log informational message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 */
void
FileLogger::log_info(const char *component, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  vlog_info(component, format, arg);
  va_end(arg);
}


/** Log warning message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 */
void
FileLogger::log_warn(const char *component, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  vlog_warn(component, format, arg);
  va_end(arg);
}


/** Log error message.
 * @param component component, used to distuinguish logged messages
 * @param format format of the message, see man page of sprintf for available
 * tokens.
 */
void
FileLogger::log_error(const char *component, const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  vlog_error(component, format, arg);
  va_end(arg);
}


/** Log debug message.
 * @param component component, used to distuinguish logged messages
 * @param e exception to log, exception messages will be logged
 */
void
FileLogger::log_debug(const char *component, Exception &e)
{
  if ( log_level <= LL_DEBUG ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    for (Exception::iterator i = e.begin(); i != e.end(); ++i) {
      fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s [EXCEPTION]: ", "D", now_s->tm_hour,
	      now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
      fprintf(log_file->stream(), *i);
      fprintf(log_file->stream(), "\n");
    }
    mutex->unlock();
  }
}


/** Log informational message.
 * @param component component, used to distuinguish logged messages
 * @param e exception to log, exception messages will be logged
 */
void
FileLogger::log_info(const char *component, Exception &e)
{
  if ( log_level <= LL_INFO ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    for (Exception::iterator i = e.begin(); i != e.end(); ++i) {
      fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s [EXCEPTION]: ", "I", now_s->tm_hour,
	      now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
      fprintf(log_file->stream(), *i);
      fprintf(log_file->stream(), "\n");
    }
    mutex->unlock();
  }
}


/** Log warning message.
 * @param component component, used to distuinguish logged messages
 * @param e exception to log, exception messages will be logged
 */
void
FileLogger::log_warn(const char *component, Exception &e)
{
  if ( log_level <= LL_WARN ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    for (Exception::iterator i = e.begin(); i != e.end(); ++i) {
      fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s [EXCEPTION]: ", "W", now_s->tm_hour,
	      now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
      fprintf(log_file->stream(), *i);
      fprintf(log_file->stream(), "\n");
    }
    mutex->unlock();
  }
}


/** Log error message.
 * @param component component, used to distuinguish logged messages
 * @param e exception to log, exception messages will be logged
 */
void
FileLogger::log_error(const char *component, Exception &e)
{
  if ( log_level <= LL_ERROR ) {
    gettimeofday(now, NULL);
    localtime_r(&now->tv_sec, now_s);
    mutex->lock();
    for (Exception::iterator i = e.begin(); i != e.end(); ++i) {
      fprintf(log_file->stream(), "%s %02d:%02d:%02d.%06ld %s [EXCEPTION]: ", "E", now_s->tm_hour,
	      now_s->tm_min, now_s->tm_sec, now->tv_usec, component);
      fprintf(log_file->stream(), *i);
      fprintf(log_file->stream(), "\n");
    }
    mutex->unlock();
  }
}

