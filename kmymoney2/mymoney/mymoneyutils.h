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

#include <kglobal.h>
#include <klocale.h>


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

typedef QString String;
#endif

//typedef for data type to store currency with.
typedef long long DLONG;

//class that has utility functions to use throughout the application.
class MyMoneyUtils
{
public:
	MyMoneyUtils() {};
	~MyMoneyUtils() {};
	
	//static function to add the correct file extension at the end of the file name
	static bool appendCorrectFileExt(String& str, const String strExtToUse);
};

#include <cstddef>
#define _CHECK_MEMORYPP_FILENAME_SIZE 256

typedef struct {
  void *p;
  int line;
  size_t size;
  char file[_CHECK_MEMORYPP_FILENAME_SIZE];
} _CheckMemoryData;

class _CheckMemoryHashTable {
 public:
  _CheckMemoryHashTable();
  _CheckMemoryHashTable(int nSize);
  virtual ~_CheckMemoryHashTable();

  bool Create(int nSize);
  void Destroy();
  void DeleteAll();
  inline bool IsCreated() { return (data != NULL); };
  inline int GetSize() { return size; };
  inline _CheckMemoryData *GetData() { return data; };
  inline _CheckMemoryData &operator [](int index) { return data[index]; };

  bool SetAt(const _CheckMemoryData &d);
  bool GetAt(void *p,_CheckMemoryData &d);
  bool DeleteAt(void *p);

 private:
  int Hash(void *p);
  static bool ReHash(_CheckMemoryHashTable &table);

  _CheckMemoryData *data;
  int size;
};
typedef void _CheckMemoryOutFunc(const char *);

class _CheckMemory {
 public:
  _CheckMemory();
  _CheckMemory(_CheckMemoryOutFunc *out);
  virtual ~_CheckMemory();

  _CheckMemoryOutFunc *SetOutFunc(_CheckMemoryOutFunc *out);
  bool CheckMemoryLeak(bool freeall);
  void FreeAll();
  inline void Restart() { table.DeleteAll(); };

  int TableCount(void);

 private:
  void Output(const char *fmt,...);

  _CheckMemoryHashTable table;
  _CheckMemoryOutFunc *outfunc;

  friend void * operator new(size_t s,const char *file,int line);
  friend void * operator new [] (size_t,const char *file,int line);
  friend void operator delete(void *p);
  friend void operator delete [] (void *p);
};

void * operator new(size_t s,const char *file,int line); // Normal new operator
void * operator new [] (size_t s,const char *file,int line); // Array new operator
void operator delete(void *p);
void operator delete [] (void *p);

#define new new(__FILE__,__LINE__)

extern _CheckMemory chkmem;

void _CheckMemory_Init(_CheckMemoryOutFunc *out);
void _CheckMemory_End();
#define _CheckMemory_Leak(freeall) chkmem.CheckMemoryLeak(freeall)
#define _CheckMemory_FreeAll() chkmem.FreeAll()

#endif
