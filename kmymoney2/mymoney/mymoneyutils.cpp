/***************************************************************************
                          mymoneyutils.cpp  -  description
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

#include "mymoneyutils.h"

#ifdef _CHECK_MEMORY

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <iostream>

#undef new

#undef _CheckMemory_Leak
#undef _CheckMemory_FreeAll

#define _CHECK_MEMORYPP_START_SIZE 59

_CheckMemory chkmem;
bool enable=false;

_CheckMemoryHashTable::_CheckMemoryHashTable()
{
  size=-1;
  data=NULL;
}
_CheckMemoryHashTable::_CheckMemoryHashTable(int nSize)
{
  Create(nSize);
}
_CheckMemoryHashTable::~_CheckMemoryHashTable()
{
  Destroy();
}
bool _CheckMemoryHashTable::Create(int nSize)
{
  if(!IsCreated()) {
    data = reinterpret_cast<_CheckMemoryData *>(malloc(nSize*sizeof(_CheckMemoryData)));
    if(data == NULL) return false;
    size=nSize;
    DeleteAll();
  }
  return true;
}
void _CheckMemoryHashTable::Destroy()
{
  if(IsCreated()) {
    free(data);
    size = -1;
    data = NULL;
  }
}
void _CheckMemoryHashTable::DeleteAll()
{
  register int i;
  if(IsCreated()) for(i=0;i<size;i++) data[i].p = NULL; // Set ALL to NULL!
}
bool _CheckMemoryHashTable::DeleteAt(void *p)
{
  if(IsCreated()) {
    int i=Hash(p);

    if(data[i].p == NULL) return false;
    if(data[i].p != p) return false;
    data[i].p = NULL;
    return true;
  }
  return false;
}
bool _CheckMemoryHashTable::SetAt(const _CheckMemoryData &d)
{
  int i;
  if(d.p == NULL) return false;
  if(!IsCreated()) return false;

  for(;;) {
    i = Hash(d.p);
    if(data[i].p == NULL) break;
    else if(data[i].p == d.p) return false;
    else {
      if(_CheckMemoryHashTable::ReHash(*this) == false) return false;
    }
  }
  data[i] = d;
  return true;
}
bool _CheckMemoryHashTable::GetAt(void *p,_CheckMemoryData &d)
{
  int i;
  if(!IsCreated()) return false;

  i = Hash(p);
  d = data[i];
  return true;
}
int _CheckMemoryHashTable::Hash(void *p)
{
  if(!IsCreated()) return -1;
  return((int)p%size);
}
bool _CheckMemoryHashTable::ReHash(_CheckMemoryHashTable &table)
{
  register int i;
  _CheckMemoryHashTable newt;

  if(!table.IsCreated()) return false;
  if(newt.Create(table.size*2+1)) {
    for(i=0;i<table.size;i++) {
      if(table.data[i].p != NULL) newt.SetAt(table.data[i]);
    }
    table.Destroy();
    table.data = newt.data;
    table.size = newt.size;
    newt.data = NULL;
    return true;
  }
  return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
_CheckMemory::_CheckMemory()
{
  table.Create(_CHECK_MEMORYPP_START_SIZE);
  outfunc = (_CheckMemoryOutFunc *)NULL;
}
_CheckMemory::_CheckMemory(_CheckMemoryOutFunc *out)
{
  table.Create(_CHECK_MEMORYPP_START_SIZE);
  outfunc = out;
}
_CheckMemory::~_CheckMemory()
{
  table.Destroy();
}
_CheckMemoryOutFunc *_CheckMemory::SetOutFunc(_CheckMemoryOutFunc *out)
{
  _CheckMemoryOutFunc *old;
  old = outfunc;
  outfunc = out;
  return old;
}
void _CheckMemory::Output(const char *fmt,...)
{
  va_list args;
  char buf[128];
  va_start(args,fmt);
  if(outfunc) {
    vsprintf(buf,fmt,args);
    outfunc(buf);
  }
  else {
    vfprintf(stderr,fmt,args);
    putchar('\n');
  }
  va_end(args);
}

int _CheckMemory::TableCount(void)
{
  size_t total;
  int freec = 0;
  int i;

  if(!table.IsCreated()) return 0;

  for(i=0;i<table.GetSize();i++) {
    if(table[i].p != NULL) {
      ++freec;
      total += table[i].size;
    }
  }
  return freec;
}

bool _CheckMemory::CheckMemoryLeak(bool freeall)
{
  register int i;
  if(!table.IsCreated()) return false;
  bool d;
  size_t total;
  int freec;

  total=0;
  freec=0;
  d=false;
  for(i=0;i<table.GetSize();i++) {
    if(table[i].p != NULL) {
      total+=table[i].size;
      freec++;
      if(d == false) {
	Output("CheckMemory++: CheckMemoryLeak: Memory leak detected!");
	Output("Position  |Size(bytes)  |Allocate at");
	
	d=true;
      }
      if(d==true) Output("%p |%-13d|%s:%d",table[i].p,table[i].size,table[i].file,table[i].line);
    }
  }
  if(d == true) Output("You have forgotten to free %d objects, %d bytes of memory.",freec,total);
  else Output("CheckMemory++: CheckMemoryLeak: No memory leak detected.");
  if(freeall == true) FreeAll();
  return true;
}
void _CheckMemory::FreeAll()
{
  register int i;
  size_t total=0;
  int freec=0;

  if(!table.IsCreated()) {
    Output("CheckMemory++: FreeAll: there is not hash table in the memory!");
    return;
  }
  for(i=0;i<table.GetSize();i++) {
    if(table[i].p != NULL) {
      Output("CheckMemory++: FreeAll: freed %d bytes of memory at %p.",table[i].size,table[i].p);
      free(table[i].p);
      total+=table[i].size;
      freec++;
      table[i].p = NULL;
    }
  }
  Output("CheckMemory++: FreeAll: Totally freed %d objects, %d bytes of memory.",freec,total);
}
void _CheckMemory_Init(_CheckMemoryOutFunc *out)
{
  if(enable!=true) {
    chkmem.Restart();
    chkmem.SetOutFunc(out);
    enable=true;
  }
}
void _CheckMemory_End()
{
  if(enable!=false) {
    chkmem.Restart();
    chkmem.SetOutFunc(NULL);
    enable=false;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
void *operator new(size_t s,const char *file,int line)
{
  void *p = malloc(s);

  if(p == NULL) throw;
  if(enable==true) {
    _CheckMemoryData data;

    data.p = p;
    strcpy(data.file,file);
    data.line = line;
    data.size = s;
    if(chkmem.table.SetAt(data) == false) chkmem.Output("CheckMemory++: new: failed to set %p to table!",p);
  }
  return p;
}
void * operator new [] (size_t s,const char *file,int line)
{
  void *p = malloc(s);

  if(p == NULL) throw;
  if(enable==true) {
    _CheckMemoryData data;

    data.p = p;
    strcpy(data.file,file);
    data.line = line;
    data.size = s;
    if(chkmem.table.SetAt(data) == false) chkmem.Output("CheckMemory++: new[]: failed to set %p to table!",p);

  }
  return p;
}
void operator delete(void *p)
{
  if(enable==true) chkmem.table.DeleteAt(p);
  free(p);
}
void operator delete [] (void *p)
{
  if(enable==true) chkmem.table.DeleteAt(p);
  free(p);
}

#endif // _CHECK_MEMORY

///////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Adds the file extension to the end of the file name.
*
*	@return		bool
*						- true if name was changed
*						- false if it wasn't.
*
*	@todo			This function should be moved to a separate file, or utility file somewhere
*						in the library files, because it appears in numerous places.
*/
///////////////////////////////////////////////////////////////////////////////////////////////
bool MyMoneyUtils::appendCorrectFileExt(String& str, const String strExtToUse)
{
	/*if(!str.isEmpty())
  {
		//find last . delminator
		int nLoc = str.findRev('.');
    if(nLoc != -1)
		{
			QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
			strExt = str.right(str.length() - (nLoc + 1));
			if(strExt.find(strExtToUse, 0, FALSE) == -1)
			{
				//append to make complete file name
				strTemp.append(strExtToUse);
				str = strTemp;
			}
			else
			{
				return false;
			}
		}
		else
		{
			str.append(".");
			str.append(strExtToUse);
		}
	}
	else
	{
		return false;
	}
  */
	return true;
}


