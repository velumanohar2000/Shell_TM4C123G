# Bare-Metal UART Shell Parser for TM4C123GH6PM

## Overview

This project implements a simple shell-style command parser for the TM4C123 microcontroller (Tiva C series) running without an RTOS or standard libraries. It reads user input over UART0, parses commands into fields, and executes simple commands like `set` and `alert`.

## Features

- Interrupt-free UART polling input (`getcUart0`, `putcUart0`)
- Command parsing using a null-terminated buffer
- Field recognition (`a` for alphabetic, `n` for numeric)
- Supports up to 5 fields per command
- Commands:
  - `set <num1> <num2>`: adds two numbers
  - `alert <message>`: prints a message

## Hardware Requirements

- EK-TM4C123GXL LaunchPad (Tiva C)
- USB to Serial interface (usually through onboard ICDI)
- Terminal program (like PuTTY, TeraTerm, minicom) set to:
  - **Baud Rate:** 115200
  - **Data bits:** 8
  - **Stop bits:** 1
  - **Parity:** None
  - **Flow Control:** None

## Project Structure

- `main.c`: Contains all logic, including input, parsing, and command execution
- `uart0.c/h`: UART driver functions (not included here, must be written or provided)
- `clock.c/h`: System clock setup to 40 MHz
- `tm4c123gh6pm.h`: Header file for hardware register access (provided by TI)

## Usage

### 1. Build and Flash
Build the project using **Keil**, **Code Composer Studio**, or **arm-none-eabi-gcc**, and flash it to your EK-TM4C123GXL board.

### 2. Open Serial Terminal
Open a terminal connected to the UART0 COM port at 115200 baud.

### 3. Type Commands

#### Example 1: set 5 10
`Output: 15`
#### Example 2: alert warning
`Output: warning`