#!/usr/bin/python
# -*- coding: utf-8 -*-
#
#***************************************************************************
#                      csvpricesqif.py  -  description
#                         -------------------
# begin                 : Sat 24 Oct. 2009
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

import csv

#   *** NOTE ***  
#   It may be necessary to remove the second line, before running.

#   Simple script to convert a csv format Prices file, as from later 
#   editions of Quicken, to qif format for KMyMoney2.
#   It and its data files are expected to be in the same directory.
#   First make the script executable:- chmod u+x csvpricesqif.py
#   Run by ''./csvpricesqif.py' .
#   You will be prompted to enter input and output filenames, followed 
#   by the symbol for the stock.

#   Input format    - "23.12.09","61.62",,,
#   Output format -
#                          !Type:Prices
#                          "HJU8.BE",61.62,"23.12.09"
#                          ^

fin = raw_input('Please enter the input Prices filename (.csv, .PRN, etc.) : ')
fout = raw_input('Please enter the output filename (add .qif) : ')
symbol = raw_input('Please enter the symbol for this stock: ')
symbol ='"'+ symbol+'"'#         Add "  " around symbol

inputfile = csv.reader(open(fin, 'rb'))
outputfile = open(fout, 'w')
inputfile.next() # Skip header line.  Comment out if no header.
inputfile_list = []
inputfile_list.extend(inputfile)

for data in inputfile_list:
    line =   '!Type:Prices\n'
    line = line +symbol +',' + data[1]  +  ',"' + data[0] + '"\n' + '^\n'
    
    #print line
    outputfile.write(line)

outputfile.close()
