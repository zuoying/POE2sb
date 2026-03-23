cmake_minimum_required(VERSION 3.13)

set(PICO_PLATFORM rp2350 CACHE STRING "Platform")
set(PICO_BOARD pico2 CACHE STRING "Board")

set(TINYUSB_HOST_HCD "pico_pio_usb" CACHE STRING "TinyUSB Host HCD")

if (DEFINED ENV{PICO_PIO_USB_PATH})
    set(PICO_PIO_USB_PATH $ENV{PICO_PIO_USB_PATH})
else()
    set(PICO_PIO_USB_PATH ${PROJECT_SOURCE_DIR}/pico-pio-usb)
endif()

include(pico_sdk_import.cmake)

project(poe2gamepad C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_definitions(PICO_RP2350=1)

if (EXISTS ${PICO_PIO_USB_PATH}/CMakeLists.txt)
    add_subdirectory(${PICO_PIO_USB_PATH} pico_pio_usb)
endif()

if (EXISTS ${PICO_PIO_USB_PATH}/src/hcd_pio_usb.c)
    set(HCD_PIO_USB_SRC ${PICO_PIO_USB_PATH}/src/hcd_pio_usb.c)
elseif (EXISTS ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c)
    set(HCD_PIO_USB_SRC ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c)
endif()

add_executable(poe2gamepad
    main.c
    usb_descriptors.c
    ${HCD_PIO_USB_SRC}
)

target_include_directories(poe2gamepad PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PICO_PIO_USB_PATH}/src
)

target_link_libraries(poe2gamepad
    pico_stdlib
    tinyusb_device
    tinyusb_host
    pico_pio_usb
    hardware_pio
)

target_compile_definitions(poe2gamepad PRIVATE
    PIO_USB_HOST_D_PLUS_PIN=12
    PIO_USB_HOST_D_MINUS_PIN=13
)

pico_enable_stdio_usb(poe2gamepad 0)
pico_enable_stdio_uart(poe2gamepad 1)

pico_add_extra_outputs(poe2gamepad)
