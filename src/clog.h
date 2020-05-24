/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
//#include <stdarg.h>
#include <string>

typedef void (*log_LockFn)(void* udata, int lock);
enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
enum DOM_TYPE {DOM_SIZE,DOM_DATE};
#define MAX_LOG_FILE_SIZE (1024)

class CLog
{
public:
   CLog();
   ~CLog();

   static CLog& ins() {
      static CLog instance;
      return instance;
   }

public:
   void log_set_udata(void* udata);
   void log_set_lock(log_LockFn fn);
   void log_set_level(int level);
   void log_set_quiet(bool enable);
   void set_dom_type(DOM_TYPE type);

   void log_log(int level, const char* fmt, ...);
private:
   std::string current_filename() const;
   bool is_newlog_required();
   void open_new_file();
   void lock();
   void unlock();

private:
   HANDLE _std_handle = 0;
   void* _udata;
   log_LockFn _lock;
   FILE* _fp;
   int _level;
   bool _quiet;
   std::string _filename;
   DOM_TYPE _dom_type;
   int _last_day;
};

int get_file_size(const char* filename);

#define log_trace(...) CLog::ins().log_log(LOG_TRACE, __VA_ARGS__)
#define log_debug(...) CLog::ins().log_log(LOG_DEBUG, __VA_ARGS__)
#define log_info(...)  CLog::ins().log_log(LOG_INFO,  __VA_ARGS__)
#define log_warn(...)  CLog::ins().log_log(LOG_WARN,  __VA_ARGS__)
#define log_error(...) CLog::ins().log_log(LOG_ERROR, __VA_ARGS__)
#define log_fatal(...) CLog::ins().log_log(LOG_FATAL, __VA_ARGS__)

#endif
