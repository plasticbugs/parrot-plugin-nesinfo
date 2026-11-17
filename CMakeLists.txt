# NES Info: the first plugin that opens a file. Pick a .nes in the menu and
# press A: the TV shows its header and the ROM's CRC-32. RESET goes back to
# the menu. Also the SDK's example (sdk/firmware/revc/sdk/README.md).
#
#   cmake -B build -G Ninja -DPICO_SDK_PATH=$HOME/pico-sdk
#   ninja -C build                      -> build/NESINFO.CVP
#
# Copy NESINFO.CVP into the card's /PLUGINS folder.
cmake_minimum_required(VERSION 3.13)

# the cart's SDK: the copy in sdk/ (sdk/README.md)
set(PARROTCART_SDK ${CMAKE_CURRENT_LIST_DIR}/sdk/firmware/revc)
set(PICO_BOARD_HEADER_DIRS ${PARROTCART_SDK}/cvcart_revc/boards)
set(PICO_BOARD cvcart_revc)
set(PICO_PLATFORM rp2350-arm-s)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/cmake/pico_sdk_import.cmake)

project(nesinfo C CXX ASM)
pico_sdk_init()
include(${PARROTCART_SDK}/sdk/cvplugin.cmake)

# The same source makes the same file, whenever it's built: the compiler's
# __DATE__ and __TIME__ (the build date in the binary info) come from
# SOURCE_DATE_EPOCH, fixed here at the tested build's, 2026-11-15 19:28:57 UTC.
set(BUILD_EPOCH 1794770937)
set(CMAKE_C_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=${BUILD_EPOCH})
set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env SOURCE_DATE_EPOCH=${BUILD_EPOCH})

add_executable(nesinfo src/nesinfo.c src/nesread.c src/nesui.c src/ines.c)
target_include_directories(nesinfo PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
cvkernel_use(nesinfo)
cvsd_use(nesinfo)
pico_enable_stdio_usb(nesinfo 0)
pico_enable_stdio_uart(nesinfo 0)
cvcart_plugin(nesinfo ID parrotcart.nesinfo NAME "NES Info" VERSION 1.0.1 BY "Parrot Cart"
              HOME /NES OPENS .nes ICON src/nesinfo_icon.txt FILE_ICON src/nes_file_icon.txt
              README CART-README.md)
