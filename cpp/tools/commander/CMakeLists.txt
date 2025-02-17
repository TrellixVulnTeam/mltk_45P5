project(mltk_commander
        VERSION 1.0.0
        DESCRIPTION "Silab's Commander Utility"
)
export(PACKAGE ${PROJECT_NAME})


macro(mltk_commander_add_download_target target)
  mltk_load_python()
  mltk_get(MLTK_PLATFORM_NAME)
  mltk_get(MLTK_JLINK_DEVICE)

  if(MLTK_JLINK_DEVICE)
    set(device_arg --device ${MLTK_JLINK_DEVICE})
  endif()

  add_custom_target(${target}_download
    COMMAND ${PYTHON_EXECUTABLE} "${MLTK_DIR}/utils/commander/commander.py" --platform ${MLTK_PLATFORM_NAME} --path "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.s37" --masserase ${device_arg}
    DEPENDS ${target}
    COMMENT "Downloading ${target}.s37 to ${MLTK_PLATFORM_NAME}'s flash ..."
    USES_TERMINAL
  )

endmacro()


macro(mltk_commander_add_reset_target target)
  mltk_load_python()
  mltk_get(MLTK_PLATFORM_NAME)
  mltk_get(MLTK_JLINK_DEVICE)

  if(MLTK_JLINK_DEVICE)
    set(device_arg --device ${MLTK_JLINK_DEVICE})
  endif()

  add_custom_target(${target}_reset
    COMMAND ${PYTHON_EXECUTABLE} "${MLTK_DIR}/utils/commander/commander.py" --platform ${MLTK_PLATFORM_NAME} --reset ${device_arg}
    COMMENT "Resetting ${MLTK_PLATFORM_NAME} ..."
    USES_TERMINAL
  )

endmacro()


###########################################################
# mltk_commander_add_download_run_target
#
# Run the given target's firmware application on an embedded device.
#
# target - CMake target name of the embedded executable
#
# Optional arguments:
# TIMEOUT <seconds> - Number of seconds to wait before timing out when using the download_run target
# COMPLETED_MSG <msg> - Console msg app prints to be considered complete
macro(mltk_commander_add_download_run_target target)
  cmake_parse_arguments(DOWNLOAD_RUN_ARG "" "TIMEOUT;COMPLETED_MSG" "" ${ARGN})
  mltk_load_python()
  mltk_get(MLTK_PLATFORM_NAME)
  mltk_get(MLTK_JLINK_DEVICE)
  mltk_get(MLTK_DOWNLOAD_RUN_SETUP_SCRIPT)
  mltk_get(MLTK_DOWNLOAD_RUN_SETUP_SCRIPT_ARGS)
  mltk_get(MLTK_DOWNLOAD_RUN_PORT)
  mltk_get(MLTK_DOWNLOAD_RUN_BAUD)
  mltk_get(MLTK_DOWNLOAD_RUN_TIMEOUT)
  mltk_get(MLTK_DOWNLOAD_RUN_SSH_HOSTNAME)
  mltk_get(MLTK_DOWNLOAD_RUN_NO_MASSERASE)
  mltk_get(MLTK_DOWNLOAD_RUN_COMPLETED_MSG)

  if(MLTK_JLINK_DEVICE)
    set(device_arg --device ${MLTK_JLINK_DEVICE})
  endif()

  if(MLTK_DOWNLOAD_RUN_SETUP_SCRIPT)
    set(setup_script_arg --setup-script "${MLTK_DOWNLOAD_RUN_SETUP_SCRIPT}")
  endif()

  if(MLTK_DOWNLOAD_RUN_SETUP_SCRIPT_ARGS)
    set(setup_script_args_arg --setup-script-args "${MLTK_DOWNLOAD_RUN_SETUP_SCRIPT_ARGS}")
  endif()

  if(MLTK_DOWNLOAD_RUN_PORT)
    set(port_arg --port "${MLTK_DOWNLOAD_RUN_PORT}")
  endif()

  if(MLTK_DOWNLOAD_RUN_BAUD)
    set(baud_arg --baud ${MLTK_DOWNLOAD_RUN_BAUD})
  endif()

  if(DOWNLOAD_RUN_ARG_TIMEOUT)
    set(timeout_arg --timeout ${DOWNLOAD_RUN_ARG_TIMEOUT})
  elseif(MLTK_DOWNLOAD_RUN_TIMEOUT)
    set(timeout_arg --timeout ${MLTK_DOWNLOAD_RUN_TIMEOUT})
  endif()

  if(MLTK_DOWNLOAD_RUN_SSH_HOSTNAME)
    set(host_arg --host "${MLTK_DOWNLOAD_RUN_SSH_HOSTNAME}")
  endif()

  if(DOWNLOAD_RUN_ARG_COMPLETED_MSG)
    set(completed_msg_arg --completed-msg "${DOWNLOAD_RUN_ARG_COMPLETED_MSG}")
  elseif(MLTK_DOWNLOAD_RUN_COMPLETED_MSG)
    set(completed_msg_arg --completed-msg "${MLTK_DOWNLOAD_RUN_COMPLETED_MSG}")
  endif()

  if(NOT MLTK_DOWNLOAD_RUN_NO_MASSERASE)
    set(masserase_arg --masserase)
  endif()

  add_custom_target(${target}_download_run
    COMMAND ${PYTHON_EXECUTABLE} -m mltk build download_run "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${target}.s37" --platform ${MLTK_PLATFORM_NAME} ${masserase_arg} ${device_arg} ${setup_script_arg} ${setup_script_args_arg} ${port_arg} ${baud_arg} ${timeout_arg} ${host_arg} ${completed_msg_arg}
    DEPENDS ${target}
    COMMENT "Downloading and running ${target}.s37 on ${MLTK_PLATFORM_NAME}'s ..."
    USES_TERMINAL
  )

endmacro()