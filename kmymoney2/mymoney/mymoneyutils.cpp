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
#include "mymoneyaccount.h"

#ifdef _CHECK_MEMORY

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <iostream>

#undef new
#undef _CheckMemory_Leak
#undef _CheckMemory_FreeAll

_CheckMemory chkmem;
bool enable=false;

_CheckMemoryEntry::_CheckMemoryEntry(void *p, int line, size_t size, const char *file)
  : m_p(p), m_line(line), m_size(size), m_file(file)
{
}

_CheckMemoryEntry::_CheckMemoryEntry()
  : m_p(0), m_line(0), m_size(0)
{
}


/////////////////////////////////////////////////////////////////////////////////////////
_CheckMemory::_CheckMemory()
{
  outfunc = (_CheckMemoryOutFunc *)NULL;
}

_CheckMemory::_CheckMemory(_CheckMemoryOutFunc *out)
{
  outfunc = out;
}

_CheckMemory::~_CheckMemory()
{
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
    putc('\n', stderr);
  }
  va_end(args);
}

int _CheckMemory::TableCount(void)
{
  return table.size();
}

bool _CheckMemory::CheckMemoryLeak(bool freeall)
{
  bool d = false;
  size_t total = 0;
  int freec = 0;
  CheckMemoryTable::ConstIterator it;

  for(it = table.begin(); it != table.end(); ++it) {
    if((*it).pointer() != 0) {
      total += (*it).size();
      freec++;
      if(d == false) {
        Output("CheckMemory++: CheckMemoryLeak: Memory leak detected!");
        Output("Position  |Size(bytes)  |Allocated at");
        d=true;
      }
      if(d==true)
        Output("%p |%-13d|%s:%d",(*it).pointer(),(*it).size(),(*it).file(),(*it).line());
    }
  }
  if(d == true)
    Output("You have forgotten to free %d object(s), %d bytes of memory.",freec,total);
  else
    Output("CheckMemory++: CheckMemoryLeak: No memory leak detected.");
  if(freeall == true)
    FreeAll();
  return true;
}

void _CheckMemory::FreeAll()
{
  size_t total=0;
  int freec=0;
  CheckMemoryTable::Iterator it;

  for(it = table.begin(); it != table.end(); it = table.begin()) {
    if((*it).pointer() != 0) {
      total += (*it).size();
      freec++;
      Output("CheckMemory++: FreeAll: freed %d bytes of memory at %p.",(*it).size(),(*it).pointer());
      free((*it).pointer());
    }
    table.remove(it);
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
    _CheckMemoryEntry entry(p, line, s, file);
    chkmem.table[p] = entry;
  }
  return p;
}

void * operator new [] (size_t s,const char *file,int line)
{
  void *p = malloc(s);

  if(p == NULL) throw;
  if(enable==true) {
    _CheckMemoryEntry entry(p, line, s, file);
    chkmem.table[p] = entry;
  }
  return p;
}

void operator delete(void *p)
{
  if(enable==true) {
    CheckMemoryTable::Iterator it;
    it = chkmem.table.find(p);
    if(it != chkmem.table.end()) {
      chkmem.table.remove(it);
    }
  }
  free(p);
}

void operator delete [] (void *p)
{
  if(enable==true) {
    CheckMemoryTable::Iterator it;
    it = chkmem.table.find(p);
    if(it != chkmem.table.end()) {
      chkmem.table.remove(it);
    }
  }
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
