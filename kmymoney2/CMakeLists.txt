PROJECT(KMyMoney2)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  ${KMyMoney_SOURCE_DIR}/libkdchart )

ADD_SUBDIRECTORY( mymoney ) 
ADD_SUBDIRECTORY( views )
ADD_SUBDIRECTORY( plugins ) 
ADD_SUBDIRECTORY( dialogs ) 
ADD_SUBDIRECTORY( widgets ) 
ADD_SUBDIRECTORY( converter ) 
ADD_SUBDIRECTORY( reports ) 
ADD_SUBDIRECTORY( wizards )
ADD_SUBDIRECTORY( pics ) 
ADD_SUBDIRECTORY( html ) 
ADD_SUBDIRECTORY( templates ) 

INCLUDE_DIRECTORIES( ${Dialogs_BINARY_DIR} )

SET( _HEADERS export.h kmymoneyutils.h kmymoneyglobalsettings.h )

# we call a macro that creates symbolic links in kmymoney/
KMM_CREATE_LINKS( ${_HEADERS} )
KMM_CREATE_LINKS_BIN( kmymoneysettings.h )

########### common code (kmymoney_common) STATIC ###############
# will be linked into kmymoney2, kmymoneytest, and libkmymoney.so

SET( kmymoney_common_SRCS
  kmymoneyutils.cpp kstartuplogo.cpp kmymoneyglobalsettings.cpp )

KDE3_ADD_KCFG_FILES( kmymoney_common_SRCS kmymoneysettings.kcfgc )
KDE3_AUTOMOC( ${kmymoney_common_SRCS} )

ADD_LIBRARY(kmymoney_common STATIC ${kmymoney_common_SRCS})
TARGET_LINK_LIBRARIES(kmymoney_common
  ${QT_AND_KDECORE_LIBS} kmm_mymoney)

# must build kmymoney/transactionsortoption.h
# from transactionsortoption.ui first
ADD_DEPENDENCIES(kmymoney_common generate_base_ui_srcs)

SET( kmymoney_link_LIBS
  ${QT_AND_KDECORE_LIBS} views reports kmymoney_common
  newuserwizard newaccountwizard wizardpages dialogs settings widgets
  converter kmm_storage kmm_mymoney kgpgfile interfaces kmm_plugin
  dialogs kmm_kdchart khtml kdeui kabc )

########### kmymoney2 executable ###############

SET( kmymoney2_SRCS main.cpp kmymoney2.cpp )

KDE3_ADD_DCOP_SKELS( kmymoney2_SRCS kmymoney2.h )
KDE3_ADD_DCOP_STUBS( kmymoney2_SRCS kmymoney2.h )
KDE3_AUTOMOC( ${kmymoney2_SRCS} )

KDE3_ADD_EXECUTABLE( kmymoney2 ${kmymoney2_SRCS} )
TARGET_LINK_LIBRARIES( kmymoney2 ${kmymoney_link_LIBS} )

########### kmymoneytest executable ###############

# shall we build the tests? can be set with ccmake.
IF( KDE3_BUILD_TESTS )

  # the kmymoney sources are used here, too.
  # but additionally, we need all test sources.

  # we try to be clever to find the test-only sources.
  # the naming convention for all test-related cpp-files is *test.cpp
  file(GLOB_RECURSE kmymoneytest_SRCS *test.cpp)
  # also find the headers, but only those, that have .cpp files
  SET (kmymoneytest_HDRS)
  FOREACH(cpp_FILE ${kmymoneytest_SRCS})
    get_filename_component( abs_path ${cpp_FILE} PATH)
    get_filename_component( stem ${cpp_FILE} NAME_WE)
    SET( hdr_candidate "${abs_path}/${stem}.h" )
    IF ( EXISTS ${hdr_candidate} )
      SET (kmymoneytest_HDRS ${kmymoneytest_HDRS} ${hdr_candidate} )
    ENDIF ( EXISTS ${hdr_candidate} )
  ENDFOREACH(cpp_FILE)

  SET( kmymoneytest_SRCS ${kmymoneytest_SRCS} reports/reportstestcommon.cpp )
  KDE3_AUTOMOC( ${kmymoneytest_SRCS} )

  KDE3_ADD_EXECUTABLE( kmymoneytest ${kmymoneytest_SRCS} )
  TARGET_LINK_LIBRARIES( kmymoneytest
  ${kmymoney_link_LIBS} ${CPPUNIT_LIBRARY} )

  ##################
  # The Actual Testing using CTest


  # this code will automatically go through all *test.h header files.
  # in each file, it expects at most one declaration of the form
  # CPPUNIT_TEST_SUITE( test_suite )
  # additionally, it expects an arbitrary number of individual tests
  # CPPUNIT_TEST ( single_test )
  # it then creates the correct global name test_suite::single_test
  # for all tests and adds it to the list all_tests

  # make sure this is empty
  SET( all_tests )
  FOREACH( _hdr_FILE ${kmymoneytest_HDRS} )
    FILE( READ ${_hdr_FILE} _contents )# read the header file

    # first find the name of the test_suite and write it to ${test_suite}
    STRING( REGEX MATCH "CPPUNIT_TEST_SUITE *\\( *([^ )]+) *\\)"
      _ign "${_contents}" )
    SET(test_suite ${CMAKE_MATCH_1})
    #message ("${test_suite}") #debug

    # now find all lines containing tests
    STRING( REGEX MATCHALL "CPPUNIT_TEST *\\( *([^ )]+) *\\)"
      _tests_in_this_suite "${_contents}" )

    # for each line, extract the name ${single_test} of the test
    FOREACH( _single_test ${_tests_in_this_suite} )
      STRING( REGEX MATCH "CPPUNIT_TEST *\\( *([^ )]+) *\\)"
        _ign "${_single_test}" )
      SET(single_test ${CMAKE_MATCH_1})

      # add the global name of the test to the list of all tests.
      SET( global_name "${test_suite}::${single_test}" )
      #message (${global_name}) #debug
      SET( all_tests ${all_tests} "${global_name}")
    ENDFOREACH( _single_test )
  ENDFOREACH( _hdr_FILE )

  # now add the test to the ctest environment.
  FOREACH( test ${all_tests} )
    ADD_TEST( "${test}" "kmymoneytest" "${test}" )
    # first argument ${test}: name of the test used for ctest
    # second argument: executable to run the test (from kmymoneytest.cpp)
    # third argument: command line argument for the executable
  ENDFOREACH( test )

  ADD_CUSTOM_TARGET( test
    COMMAND "ctest" "${CMAKE_CURRENT_SOURCE_DIR}")
ENDIF( KDE3_BUILD_TESTS )


########### install files ###############

INSTALL(TARGETS kmymoney2 DESTINATION bin)
INSTALL(CODE
  "EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} -E create_symlink kmymoney2 kmymoney
  WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/)"
  )
INSTALL(FILES kmymoney2.kcfg
  DESTINATION share/config.kcfg 
  )
INSTALL(FILES kmymoney2ui.rc tips
  DESTINATION share/apps/kmymoney2 
  )
INSTALL(FILES ${_HEADERS}
  DESTINATION include/kmymoney
  )
INSTALL(FILES x-kmymoney2.desktop
  DESTINATION ${KDE3MIMEDIR}/applications
  )
INSTALL(FILES kmymoney2.desktop
  DESTINATION share/applications/kde
  )
INSTALL(FILES misc/financequote.pl
  DESTINATION share/apps/kmymoney2/misc
  )

########## Application Icons #############
INSTALL(DIRECTORY icons/hicolor
  DESTINATION share/apps/kmymoney2/icons
  PATTERN "CVS" EXCLUDE
  PATTERN ".cvsignore" EXCLUDE
  PATTERN "*Make*" EXCLUDE
  )
INSTALL(FILES hi16-app-kmymoney2.png
  DESTINATION share/icons/hicolor/16x16/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES mimetype_kmy_16x16.png
  DESTINATION share/icons/hicolor/16x16/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi32-app-kmymoney2.png
  DESTINATION share/icons/hicolor/32x32/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES mimetype_kmy_22x22.png
  DESTINATION share/icons/hicolor/22x22/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES mimetype_kmy_32x32.png
  DESTINATION share/icons/hicolor/32x32/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi48-app-kmymoney2.png
  DESTINATION share/icons/hicolor/48x48/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES mimetype_kmy_48x48.png
  DESTINATION share/icons/hicolor/48x48/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi64-app-kmymoney2.png
  DESTINATION share/icons/hicolor/68x68/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES mimetype_kmy_64x64.png
  DESTINATION share/icons/hicolor/64x64/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES hi128-app-kmymoney2.png
  DESTINATION share/icons/hicolor/128x128/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES mimetype_kmy_128x128.png
  DESTINATION share/icons/hicolor/128x128/mimetypes
  RENAME kmy.png
  )
INSTALL(FILES lo16-app-kmymoney2.png
  DESTINATION share/icons/locolor/16x16/apps
  RENAME kmymoney2.png
  )
INSTALL(FILES lo32-app-kmymoney2.png
  DESTINATION share/icons/locolor/32x32/apps
  RENAME kmymoney2.png
  )
