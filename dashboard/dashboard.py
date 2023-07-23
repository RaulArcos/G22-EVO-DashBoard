from gps import GPS
from can_bus import CanBus
from excel_manager import ExcelManager
from dash_screen_controller import DashScreenController
import datetime
import threading
import os

if __name__ == "__main__":
    
    gps_port = "/dev/ttyS0"
    can_bus_port = "can0"

    file_directory = os.path.dirname(os.path.abspath(__file__))
    excel_file_path = file_directory + "/gps_logs/gps_data_" + datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S") + ".xlsx"
    excel_manager = ExcelManager(excel_file_path)
    excel_manager.create_excel_file()

    gps = GPS(gps_port, excel_manager)
    can = CanBus()

    gps_thread = threading.Thread(target=gps.receive_gps_data)
    gps_thread.start()
    
    can.connect_can_bus(can_bus_port, 'socketcan')
    can_thread = threading.Thread(target=can.receive_can_bus_data)
    can_thread.start()

    