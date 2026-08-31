# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

mcux_add_configuration(
    CC "-DSDK_DEBUGCONSOLE=1"
    CX "-DSDK_DEBUGCONSOLE=1"
)

mcux_add_source(
    SOURCES board/board.c
            board/board.h
            board/FreeRTOSConfigBoard.h
            board/se_board_config.h
)

mcux_add_include(
    INCLUDES board
)

mcux_add_source(
    SOURCES board/clock_config.c
            board/clock_config.h
)

mcux_add_include(
    INCLUDES board
)

mcux_add_source(
    SOURCES board/pin_mux.c
            board/pin_mux.h
)

mcux_add_include(
    INCLUDES board
)

mcux_add_source(
    SOURCES board/peripherals.c
            board/peripherals.h
            board/RTE_Device.h
)

mcux_add_include(
    INCLUDES board
)

mcux_add_source(
    SOURCES app/app.h
            app/hardware_init.c
)

mcux_add_include(
    INCLUDES app
)

