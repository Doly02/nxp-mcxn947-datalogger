#
#   Project:        NXP MCXN947 Datalogger
#   Author:         Tomas Dolak
#   File:           serial_tests.py
#   Description:    This File Contains Functional Test of Digital Data Logger, The Script Loads The Test Files and Sends Them Thru
#                   COM Port To Digital Datalogger. 
#   

# Libraries 
import serial
import time
import os

# Configuration
TESTS_FOLDER    = 'test_files'  # Relative path to the test folder
TEST_CASES      = ['test_1kb_one_line.txt','test_420b.txt','test_ahoj.txt']

COM_PORT        = 'COM11'
BAUD_RATE       = 230400


class SerialTest:
    """
    Class for handling UART communication and preparing data for testing.

    Attributes:
        com_port (str): COM port name (e.g., 'COM3', '/dev/ttyUSB0').
        baud_rate (int): UART baud rate.
    """

    def __init__(self, com_port='COM11', baud_rate=230400):
        """
        Initialize the SerialTest class.

        :param com_port: COM port name, e.g. 'COM3' on Windows or '/dev/ttyUSB0' on Linux (default is 'COM3').
        :param baud_rate: UART baud rate, e.g. 9600, 11520 (default is 320400).
        """
        self.com_port = com_port
        self.baud_rate = baud_rate
        # Locate the folder where this script is located and append the test folder
        script_dir = os.path.abspath(os.path.dirname(__file__))
        self.folder_path = os.path.join(script_dir, TESTS_FOLDER)


    def send_uart_data(self, data):
        """
        Connects to the COM port and sends data to the UART.

        :param data: Data to send (as a string or bytes).
        """
        try:
            with serial.Serial(
                port=self.com_port,
                baudrate=self.baud_rate,
                bytesize=serial.EIGHTBITS,  # Data bits (EIGHTBITS = 8)
                parity=serial.PARITY_NONE,  # No parity
                stopbits=serial.STOPBITS_ONE,  # 1 stop bit
                timeout=1  # Timeout in seconds
            ) as port:
                print(f"\033[92m[SUCCESS]\033[0m Connected to {self.com_port} at {self.baud_rate} baud.")

                # Send Data
                if isinstance(data, str):
                    data = data.encode()  # Convert string to bytes

                port.write(data)
                port.flush()
                print(f"Data Sent")

        except serial.SerialException as e:
            print(f"Serial communication error: {e}")

    def prepare_data(self, file):
        """
        Reads the file from the test folder based on file name and returns its content.

        :param file: Filename of the test file to send .
        :return: File content as a string, or None if no files are found.
        """
        try:
            file_path = os.path.join(self.folder_path, file)

            if not os.path.isfile(file_path):
                print(f"File not found: {file_path}")
                return None

            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                print(f"Read content from file: {file_path}")
                return content

        except FileNotFoundError:
            print(f"Folder not found: {self.folder_path}")
            return None
        except Exception as e:
            print(f"Error reading file: {e}")
            return None

if __name__ == "__main__":
    # Initialize the SerialTest class
    serial_test = SerialTest(com_port=COM_PORT, baud_rate=BAUD_RATE)

    # Prepare data to send
    for test in TEST_CASES:
        print(f"Sending data from file: {test}")
        data = serial_test.prepare_data(test)
        
        serial_test.send_uart_data(data)
        time.sleep(2)

    # Send data to the UART
    # serial_test.send_uart_data(data)
