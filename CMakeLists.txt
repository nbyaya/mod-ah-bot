#
#
#

# Set path
CU_SET_PATH("CMAKE_MOD_AHBOT_DIR" "${CMAKE_CURRENT_LIST_DIR}")

# CMake hooks
CU_ADD_HOOK(AFTER_LOAD_CONF "${CMAKE_CURRENT_LIST_DIR}/cmake/after_load_conf.cmake")
CU_ADD_HOOK(BEFORE_GAME_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/cmake/before_game_lib.cmake")
CU_ADD_HOOK(AFTER_SCRIPTS_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/cmake/after_scripts_lib.cmake")
CU_ADD_HOOK(AFTER_GAME_LIBRARY "${CMAKE_CURRENT_LIST_DIR}/cmake/after_game_lib.cmake")

# Add SC
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/cs_ah_bot.cpp")

# Add SL
AC_ADD_SCRIPT_LOADER("AHBotCommand" "${CMAKE_CURRENT_LIST_DIR}/src/loader_cs_ah_bot.h")

# Add conf file
AC_ADD_CONFIG_FILE("${CMAKE_MOD_AHBOT_DIR}/conf/mod_ahbot.conf.dist")
