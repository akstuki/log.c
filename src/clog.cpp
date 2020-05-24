/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include <Windows.h>

#include "clog.h"
//# define LOG_USE_COLOR


static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const int TEXT_COLOR[] = {
    FOREGROUND_BLUE      ,
     FOREGROUND_BLUE      ,
 FOREGROUND_BLUE      ,
 FOREGROUND_GREEN     ,
 FOREGROUND_RED       ,
 FOREGROUND_INTENSITY
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

CLog::CLog()
   :_level(LOG_TRACE),_lock(0),_quiet(false),_udata(0),_filename(""),_fp(nullptr),_dom_type(DOM_DATE),_last_day(-1)
{
   open_new_file();
};

CLog::~CLog() {
   if (_fp) {
      fclose(_fp); _fp = nullptr;
   }
}

void CLog::lock() {
   if (_lock) {
      _lock(_udata, 1);
   }
}


void CLog::unlock() {
   if (_lock) {
      _lock(_udata, 0);
   }
}

void CLog::log_set_udata(void *udata) {
  _udata = udata;
}


void CLog::log_set_lock(log_LockFn fn) {
  _lock = fn;
}

void CLog::log_set_level(int level) {
  _level = level;
}


void CLog::log_set_quiet(bool enable) {
  _quiet = enable;
}

void CLog::set_dom_type(DOM_TYPE type)
{
   _dom_type = type;
}

std::string CLog::current_filename() const {
   time_t t = time(NULL);
   struct tm* lt = localtime(&t);
   char buff[MAX_PATH] = { 0 };
   sprintf(buff, "%d-%d-%d %d-%d-%d.trace",lt->tm_year+1900,lt->tm_mon+1,lt->tm_mday,lt->tm_hour,lt->tm_min,lt->tm_sec);
   return std::string(buff);
}

void CLog::open_new_file() {
   if (_fp) {
      fclose(_fp); _fp = nullptr;
   }
   _filename = current_filename();
   _fp = fopen(_filename.c_str(), "a+");
}

bool CLog::is_newlog_required() {
   if (_filename.empty())
   {
      return true;
   }
   if(_dom_type==DOM_SIZE){
      return (get_file_size(_filename.c_str()) > MAX_LOG_FILE_SIZE);
   }
   if (_dom_type == DOM_DATE) {
      time_t t = time(NULL);
      struct tm* lt = localtime(&t);
      if (lt->tm_min != _last_day) {
         _last_day = lt->tm_mday;
         return true;
      }
   }

   return false;
}


void CLog::log_log(int level, const char *fmt, ...) {
  if (level < _level) {
    return;
  }

  /* Acquire lock */
  lock();

  /* Get current time */
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  /* Log to stderr */
  if (!_quiet) {
    va_list args;
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      stderr, "%s %s%-5s\x1b[0m ",
      buf, level_colors[level], level_names[level]);
#else
    if(_std_handle==0)
    _std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(_std_handle, FOREGROUND_INTENSITY | TEXT_COLOR[level]);
    fprintf(stderr, "[ %s %-5s ] ", buf, level_names[level]);
    SetConsoleTextAttribute(_std_handle, 0x07);

#endif
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  /* Log to file */
  if (get_file_size(_filename.c_str())> MAX_LOG_FILE_SIZE) {
     open_new_file();
  }
  if (_fp) {
    va_list args;
    char buf[32];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
    fprintf(_fp, "%s %-5s: ", buf, level_names[level]);
    va_start(args, fmt);
    vfprintf(_fp, fmt, args);
    va_end(args);
    fprintf(_fp, "\n");
    fflush(_fp);
  }

  /* Release lock */
  unlock();
}

int get_file_size(const char* filename)
{
   struct stat statbuf;
   stat(filename, &statbuf);
   int size = statbuf.st_size;

   return size;
}
