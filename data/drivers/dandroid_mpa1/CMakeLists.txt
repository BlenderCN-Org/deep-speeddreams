INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_SUBDIRS 0 1 2 3 4 5 6 7 8 9
                  mpa1-murasama mpa1-hartbill-2002 mpa1-furia
                  mpa1-team-lynx mpa1-ffr
                  tracks)

ROBOT_DATA(NAME dandroid_mpa1
           FILES dandroid_mpa1.xml default.xml logo.rgb readme.txt
           SUBDIRS ${ROBOT_SUBDIRS} PATTERNS *.xml logo.rgb *.png *.jpg *.csv)
