# STM32
Libraries for STM32 projects.\
These libraries were initially written for the G041 targets, but have been modified to work with the G031 variants.

This repo is a work in progress, and the code within is provided 'as is' - use at your own discretion. 

These libraries are under active maintenance, and may change without notice.

The `lib` folder contains device agnostic libraries that rely on device specific libraries.

The `libG041` folder contains the G041/G031 device specific libraries. 

OCT 09 2024 - Major fix to I2C Library, added VEML7700 library

FEB 07 2025 - Updates to many libraries to create demos. I2C lib updated to include 'unstick' code for interrupted bus.

FEB 14 2025 - Added demos.

DEMOS:

`test_g031_i2c_scan` -> demo of I2C bus scan to reveal addresses of attached devices.

`test_g031_i2c_oled_hello` -> demo of I2C use with SSD1306 compliant OLED display.

`test_g031_i2c_adxl343` -> demo of basic ADXL343 accelerometer use (no device interrupts).

`test_g031_i2c_adxl343_interrupts` -> demo of ADXL343 acceleromer using device activity interrupt line, and STM32 GPIO EXTI interrupts for activity detection.

`test_g031_timer_interrupt` -> demo of TIM2 overflow interrupt to generate a 100ms periodic interrupt for program flow control.

`test_g031_a_to_d` -> demo of A/D read.

`test_g031_timer_pwm` -> demo of PWM with TIM3::CH1, with pin output. 
