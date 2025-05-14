# Hardware Documentation

This folder contains all hardware-related materials for the expansion shield designed for the FRDM-MCXN947 development board used in data logger project.

### Folder Structure

- `expansion_shield/` – Contains the full KiCad 8 project of the expansion board. This includes the schematic, PCB layout, and associated design files.
- `BOM.csv` – A standard bill of materials generated from KiCad 8 listing the components used on the expansion board.  
- `component.txt` – A human-readable version of the BOM, listing key components along with direct links to online distributors for easy purchasing.  
  **Note:** This list does **not** include passive components such as resistors and capacitor (100 nF) used for the RTC DS3231M+.
- `schematic.pdf` – A PDF version of the complete schematic diagram of the expansion shield for quick reference without needing KiCad.

### Notes
- The KiCad project is created with **KiCad 8**. Make sure to use the correct version to avoid compatibility issues.
- The provided component links in `component.txt` were valid at the time of writing but may change over time depending on distributor availability.

For more information about the system design, refer to the `thesis/` folder or see the main `README` in root folder.