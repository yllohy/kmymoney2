/***************************************************************************
                          mymoneyutils.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MYMONEYUTILS_H_
#define _MYMONEYUTILS_H_

#include <qstring.h>
#include <qcstring.h>
#include <qvaluelist.h>

#if 0

//Includes for STL support below
#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;

#ifdef _UNICODE
typedef std::wstring String;
#else
typedef std::string String;
#endif

#else

//typedef for data type to store currency with.
typedef long long DLONG;

// typedef for id lists
typedef QValueList<QCString> QCStringList;

typedef QString String;
#endif // 0

void timetrace(char *);

//class that has utility functions to use throughout the application.
class MyMoneyUtils
{
public:
  MyMoneyUtils() {};
  ~MyMoneyUtils() {};

  //static function to add the correct file extension at the end of the file name
  static QString getFileExtension(QString strFileName);

};

class MyMoneyTracer
{
public:
  MyMoneyTracer(const QString& className, const QString& methodName);
  ~MyMoneyTracer();
  void printf(const char *format, ...) const;

  static void off(void);
  static void on(void);

private:
  QString m_className;
  QString m_memberName;

  static int m_indentLevel;
  static int m_onoff;
};

#ifdef _CHECK_MEMORY

#include <cstddef>
#include <qmap.h>

class _CheckMemoryEntry {
public:
  _CheckMemoryEntry();
  _CheckMemoryEntry(void *p, int line, size_t size, const char *file);
  ~_CheckMemoryEntry() {};

  void * pointer(void) const { return m_p; };
  const int line(void) const { return m_line; };
  const size_t size(void) const { return m_size; };
  const char* file(void) const { return m_file; };

private:
  void *m_p;
  int m_line;
  size_t m_size;
  QCString m_file;
};

typedef QMap<void *, _CheckMemoryEntry> CheckMemoryTable;

typedef void _CheckMemoryOutFunc(const char *);

class _CheckMemory {
 public:
  _CheckMemory();
  _CheckMemory(_CheckMemoryOutFunc *out);
  virtual ~_CheckMemory();

  _CheckMemoryOutFunc *SetOutFunc(_CheckMemoryOutFunc *out);
  bool CheckMemoryLeak(bool freeall);
  void FreeAll();
  inline void Restart() { table.clear(); };

  int TableCount(void);

 private:
  void Output(const char *fmt,...);

  CheckMemoryTable table;
  _CheckMemoryOutFunc *outfunc;

  friend void * operator new(size_t s,const char *file,int line) throw();
  friend void * operator new [] (size_t,const char *file,int line) throw();
  friend void operator delete(void *p) throw();
  friend void operator delete [] (void *p) throw();
};

void * operator new(size_t s,const char *file,int line) throw(); // Normal new operator
void * operator new [] (size_t s,const char *file,int line) throw(); // Array new operator
void operator delete(void *p) throw();
void operator delete [] (void *p) throw();

#define new new(__FILE__,__LINE__)

extern _CheckMemory chkmem;

void _CheckMemory_Init(_CheckMemoryOutFunc *out);
void _CheckMemory_End();
#define _CheckMemory_Leak(freeall) chkmem.CheckMemoryLeak(freeall)
#define _CheckMemory_FreeAll() chkmem.FreeAll()

#endif // _CHECK_MEMORY
#endif
