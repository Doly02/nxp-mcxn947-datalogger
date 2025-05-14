# Application – Firmware Source Code

This folder contains the complete source code, configuration, and documentation for the firmware of the digital data logger.

### Folder Structure

- `src/` – Contains all **source files** implementing the functionality of the digital data logger.  
  This includes modules for UART data reception, SD card logging, power loss detection, LED signaling, and system management.

- `include/` – Contains the corresponding **header files** for the source files in `src/`.  
  This folder also includes the main **firmware configuration file** `defs.h`, which allows developers to e.g. enable or disable specific firmware modules at compile time, adjust default values for logging parameters such as baud rate, file size, or thresholds, configuration of FreeRTOS task stacks 

- `doc/` – Contains **Doxygen-generated documentation** for the firmware in two formats:
  - `html/annotated.html` – Start page for **HTML documentation** that can be viewed in a web browser
  - `latex/refman.pdf` – Main file of the **PDF documentation** generated via LaTeX

- `configuration/` – Holds configuration files for **external modules** used in the project, such as:
  - **FreeRTOS** – Real-time operating system for task scheduling and memory management
  - **FATFS** – Lightweight filesystem used for SD card access

- Additional source files, drivers, and libraries from the **MCUXpresso SDK** are also included.  
  These are required for proper compilation and hardware support for the **NXP FRDM-MCXN947** microcontroller.

### Project Configuration
The project currently includes two build configurations:
- `Debug/`
- `Release/` 

The project structure is prepared for future expansion, where development (`Debug`) and production (`Release`) builds can be differentiated.

### Notes

- The firmware is written in **C**, structured modularly, and built for the **NXP FRDM-MCXN947** board.
- All modules and configuration options are documented in the `doc/` folder and the comments within `defs.h`.