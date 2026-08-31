
# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

cmake_path(APPEND conf_file_path ${CMAKE_CURRENT_LIST_DIR} ${board} prj.conf)
set(CONF_FILE ${conf_file_path} ${CONF_FILE})
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(main LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/main)


include(${board}/board_files.cmake)
include(${SdkRootDirPath}/CMakeLists.txt)

mcux_add_configuration(
    CC "-include ${CMAKE_CURRENT_LIST_DIR}/rte_device_override.h"
    CX "-include ${CMAKE_CURRENT_LIST_DIR}/rte_device_override.h"
)

mcux_add_source(
    SOURCES main.c
            turn_signals.c
            motors.c
)



mcux_add_source(
    BASE_PATH ${SdkRootDirPath}
    SOURCES components/cmsis_drivers/cmsis_lpuart/fsl_lpuart_cmsis.c
            components/cmsis_drivers/cmsis_lpuart/fsl_lpuart_cmsis.h
)

mcux_add_include(
    BASE_PATH ${SdkRootDirPath}
    INCLUDES components/cmsis_drivers/cmsis_lpuart
             arch/arm/CMSIS/Driver/Include
             drivers/dma3
)

mcux_add_include(
    INCLUDES frdmmcxa153/board
)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
mcux_add_config_mex_path( PATH ./frdmmcxa153 )
mcux_add_include(INCLUDES .)
