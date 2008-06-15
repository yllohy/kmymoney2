#
# this file contains the following macros:
# KMM_ADD_UI_FILES
# KMM_CREATE_LINKS

INCLUDE(AddFileDependencies)

GET_FILENAME_COMPONENT( KMM_MODULE_DIR  ${CMAKE_CURRENT_LIST_FILE} PATH)


IF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)
	FILE(MAKE_DIRECTORY ${KMyMoney_BINARY_DIR}/kmymoney)
ENDIF (NOT EXISTS ${KMyMoney_BINARY_DIR}/kmymoney)

MACRO(KMM_CREATE_LINKS)
   FOREACH(c_FILE ${ARGV})
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink  ${CMAKE_CURRENT_SOURCE_DIR}/${c_FILE} ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE})
   ENDFOREACH (c_FILE)
ENDMACRO(KMM_CREATE_LINKS)

MACRO(KMM_CREATE_LINKS_BIN)
   FOREACH(c_FILE ${ARGV})
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink  ${CMAKE_CURRENT_BINARY_DIR}/${c_FILE} ${KMyMoney_BINARY_DIR}/kmymoney/${c_FILE})
   ENDFOREACH (c_FILE)
ENDMACRO(KMM_CREATE_LINKS_BIN)

#create the implementation files from the ui files and add them to the list of sources
#usage: KMM_ADD_UI_FILES(foo_SRCS ${ui_files})
#copied from KDE3_ADD_UI_FILES
MACRO(KMM_ADD_UI_FILES _sources )
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)
      
      SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
      SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      # setting the include path to the KMyMoney plugin lib
      SET(_kmm_plugin_path ${KMyMoney2_BINARY_DIR}/widgets)

      ADD_CUSTOM_COMMAND(OUTPUT ${_header}
         COMMAND ${QT_UIC_EXECUTABLE}
         ARGS  -L ${KDE3_LIB_DIR}/kde3/plugins/designer -nounload -L ${_kmm_plugin_path} -o ${_header} ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
         DEPENDS ${_tmp_FILE}
      )

      ADD_CUSTOM_COMMAND(OUTPUT ${_src}
         COMMAND ${CMAKE_COMMAND}
         ARGS
         -DKDE_UIC_PLUGIN_DIR:FILEPATH=${KDE3_LIB_DIR}/kde3/plugins/designer
	 -DKMM_UIC_PLUGIN_DIR:FILEPATH=${_kmm_plugin_path}
         -DKDE_UIC_EXECUTABLE:FILEPATH=${QT_UIC_EXECUTABLE}
         -DKDE_UIC_FILE:FILEPATH=${_tmp_FILE}
         -DKDE_UIC_CPP_FILE:FILEPATH=${_src}
         -DKDE_UIC_H_FILE:FILEPATH=${_header}
         -P ${KMM_MODULE_DIR}/kmmuic.cmake
         DEPENDS ${_header}
      )

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_header} -o ${_moc}
         DEPENDS ${_header}
      )

      SET(${_sources} ${${_sources}} ${_src} ${_moc} )

   ENDFOREACH (_current_FILE)
ENDMACRO(KMM_ADD_UI_FILES)

#create the implementation files from the ui files and add them to the list of sources
#usage: KMM_ADD_UI_FILES(foo_SRCS ${ui_files})
# copy from KDE3_ADD_UI_FILES
MACRO(KMM_CREATE_UI_HEADER_FILES _sources )
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_absolute_current_FILE ${_current_FILE} ABSOLUTE) # absolut path to the UI File
      GET_FILENAME_COMPONENT(_basename ${_absolute_current_FILE} NAME_WE)      # only the basename without extention
      GET_FILENAME_COMPONENT(_path     ${_current_FILE} PATH)     # the path of the given file
      SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)

      # setting the include path to the KMyMoney plugin lib
      SET(_kmm_plugin_path ${KMyMoney2_BINARY_DIR}/widgets)

      ADD_CUSTOM_COMMAND(OUTPUT ${_header}
         COMMAND ${QT_UIC_EXECUTABLE}
         ARGS  -L ${KDE3_LIB_DIR}/kde3/plugins/designer -nounload -L ${_kmm_plugin_path} -o ${_header} ${_absolute_current_FILE}
         DEPENDS ${_absolute_current_FILE}
      )
 
      SET(${_sources} ${${_sources}} ${_header} )

   ENDFOREACH (_current_FILE)
ENDMACRO(KMM_CREATE_UI_HEADER_FILES)

MACRO(KMM_KDE3_ADD_KCFG_FILES  _sources _kcfg_DIR)
   FOREACH (_current_FILE ${ARGN} )
  
      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)
      
      FILE(READ ${_tmp_FILE} _contents)
      STRING(REGEX REPLACE "^(.*\n)?File=([^\n]+)\n.*$" "\\2"  _kcfg_FILE "${_contents}")

      SET(_src_FILE    ${KMyMoney2_BINARY_DIR}/${_basename}.cpp)
      SET(_header_FILE ${KMyMoney2_BINARY_DIR}/${_basename}.h)

      ADD_CUSTOM_COMMAND(OUTPUT ${_src_FILE}
         COMMAND ${KDE3_KCFGC_EXECUTABLE}
         ARGS -d ${KMyMoney2_BINARY_DIR}/ ${_kcfg_DIR}/${_kcfg_FILE} ${_tmp_FILE}
         DEPENDS ${_tmp_FILE} ${_kcfg_DIR}/${_kcfg_FILE} )

      SET(${_sources} ${${_sources}} ${_src_FILE})

   ENDFOREACH (_current_FILE)

ENDMACRO(KMM_KDE3_ADD_KCFG_FILES)

