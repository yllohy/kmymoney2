#ADD_SUBDIRECTORY( en ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


ADD_DOCS(kmymoney2 en)

########### install files ###############




#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#SUBDIRS = en
#
######## kdevelop will overwrite this part!!! (end)############
#
