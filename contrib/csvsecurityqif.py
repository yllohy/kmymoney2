#!/usr/bin/python
# -*- coding: utf-8 -*-
#
#***************************************************************************
#                      csvsecurityqif.py  -  description
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

#   Simple utility to convert a csv format Securities file, as from later 
#   editions of Quicken, to qif format for KMyMoney2.
#   It and its data files are expected to be in the same directory.
#
#   First, make the script executable: chmod u+x csvsecurityqif.py .
#   Run by './csvsecurityqif.py'.
#   You will be prompted to enter input and output filenames.
#   The entries in this input file contain a number of fields that are
#   not documented and which KMyMoney2 does not handle.
#   These fields are accepted and suffixed with 'M' in the output file.
#   Anything of importance in them will need to be copy/pasted into KMM.

fin = raw_input('Please enter the input Securities filename (.csv, .PRN, etc.) : ')
fout = raw_input('Please enter the output filename (add .qif) : ')
line = csv.reader(open(fin, "rb"))
outputfile = open(fout, 'w')
line.next() # Skip header line.  Comment out if no header.
line_list = []
line_list.extend(line)
line = "!Option:AutoSwitch\n"
outputfile.write(line)

for data in line_list:
    line = "!Account\n" + "N" +data[0]  + "\n" + "TMutual\n" + "^\n"
    line = line + "!Type:Security\n"
    line = line + "N" +data[0] + "\n"+ "S" + data[1] + "\n"+ "TUnit/Inv. Trust" #+"\n"
    line = line + "M" + data[2] + "\n"+ "M" + data[3] + "\n"+ "M" + data[4] + "\n"+ "M" + data[5] + "\n"+ "M" + data[6]
    line = line + "\n^\n"
    #print line
    outputfile.write(line)

outputfile.close()
