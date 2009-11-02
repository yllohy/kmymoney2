#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#
#***************************************************************************
#                      csvbankingqif.py  -  description
#                         -------------------
# begin                 : Sat 31 Oct. 2009
# copyright             : (C) 2009 by Allan Anderson
# email                 : aganderson@ukonline.co.uk
#
#***************************************************************************/
#
#***************************************************************************
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License as published by  *
#*   the Free Software Foundation; either version 2 of the License, or     *
#*   (at your option) any later version.                                   *
#*                                                                         *
#***************************************************************************/
#
#   											*** NOTE ***
#   It may be necessary to remove the second line, before running.
#   It may be necessary also, to change the currency symbol if your file 
#   includes one.
#
#   Simple utility to convert a csv format file, as from a bank, to qif 
#   format for KMyMoney2.  There is no standard for the layout of such a 
#   file, but generally there will be a header line which indicates the 
#   layout of the fields within the file.  Even then though, the order of 
#   the columns may vary.  It is assumed, though, that the first column 
#   will contain the date, in 'dd MM yy' format, 'MM' being the month 
#   name or number.
#   The  second column is the detail.  The third and fourth columns are 
#   assumed to be debits and credits.  Even fron the same bank, these 
#   columns may be reversed, but the script handles this.  Alternatively, 
#   the third column may be the amount.  There may also be additional 
#   columns, such as current balance, but these are all ignored.
#   Apart from the header line, there are likely to be other lines, with 
#   account number, balance details, etc.  These are skipped.
#
#   First, make the script executable: chmod u+x csvbankinyqif.py .
#   The script should be added to the KMM QIF import profile.  In KMM, open 
#   Tools/QIF Profile Editor and click on 'New' at the bottom.  then enter a 
#   name, such as csvbank, then click 'OK'.  Next, click on that name in the 
#   next window, and open the Filter tab.  For the 'Input filter location', 
#   select the location you chose for the script file.  For the Input filter 
#   file type, enter *.csv, or whatever extension your data file has.
#   Finally, click 'OK'.
#   When ready, select File/Import/QIF, and browse to your data file, then
#   select your new filter profile, and click 'Import'.
#

#desc="date","detail","debit","credit"
mnths=['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec']
currency = '£'
setup = True
debsfirst = False		#  credit column is before debit
both = False				#  only one amount column
print("!Type:Bank")
while 1:
  try:
      line=raw_input()
  except:break
  if line == "" : continue	#  empty line
  line = line.replace('"','',)	#  Strip out ' "' quotes
  line = line.replace(currency,'',)	#  Strip out '£', etc. symbol
  cols = line.split(',')     #  Seperator  between columns
  if setup:
#
#	  		*** SETUP ***
#
    dt = cols[0][0:2]	     #  Might be a date (day)
    datefound = ((dt > '0') and (dt < '32'))    #this looks like a date 
    hdrfound = (cols[0] == 'Date')
    if not datefound and not hdrfound: continue#  still in hdrs
    if hdrfound:
#
#								*** 'Date', so now in header ***
#
      hdrfound = False
      #line = line.replace(' ','',)	#  Strip out spaces in hdr
      cols[2] = cols[2].replace(' ','',)	#  Strip out spaces in hdr
      if cols[2] == 'Debits':
				debsfirst = True
				continue
      elif cols[2] == 'Credits':
	 			debsfirst = False
	 			continue
      elif cols[2] == 'Amount':
	 			both = True
	 			continue
      else:
	 			print 'Error in col[2]'
	 			print '*** Error in header - col 2 s/b Debit, Credit, or Amount'
	 			#continue
	 			exit
    setup ==False
#
#								*** Transactions ***
#
  cnum = 0                   #  First column
  for col in cols:
      if cnum > 3: break
#
#         									 #  Process Date
#
      elif  cnum == 0:
				col =col.replace(' ','/',2)  #  Change date seperator to '/'
				m = col.split('/')
#								*** Check if month not numeric
				mn = m[1][0:3]        #  Extract month string from field 2
				fld = 2
				try:
					mnth = mnths.index(mn)	#  Get month number
				except ValueError:		#  Field 2 not a valid month name
					mn = m[0][0:3]			#  .. so try field 1
					fld = 1
					try:
						mnth = mnths.index(mn)
					except ValueError:	#  Nor is field 1
						dat = ''.join(col)						#  ..so use as is (numeric)
					else:								#  Field 1 is month name
						dat = col[1:3] + str(mnth + 1) + '/' +m[2]
				else:									#  Field 2 is month name
					dat = col[0:3] + str(mnth + 1) + '/' +m[2]
				line = 'D' + dat+'\n'
#
#															#  Detail column
#
      elif cnum == 1:          
				#col = col.replace('"','')
				line = line + 'P' + col +'\n'
#
#															#  Debit or credit column
#
      elif cnum == 2:	
				if col != "":
					if debsfirst == True:		#  This is Debit column
						col = '-' + col  #  Mark as -ve
					line = line + 'T' + col +'\n'
#
#															#  Credit or debit?
#
      elif ((cnum == 3) and (both == False)):
				if col != "":
					if ((debsfirst == False) ):
						col = '-' + col  #  Mark as -ve
					line = line + 'T' + col + '\n'
      cnum+=1
  print line + '^'		#  output this entry
