from gps import GPS
from can_bus import CanBus
from excel_manager import ExcelManager
from dash_screen_controller import DashScreenController
import datetime
import threading
import os

if __name__ == "__main__":

    #Establecer el directorio de logs
    file_directory = os.path.dirname(os.path.abspath(__file__))
    excel_file_path = file_directory + "/logs/g22evo_data_" + datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S") + ".xlsx"
    excel_manager = ExcelManager(excel_file_path)
    excel_manager.create_excel_file()

    #Iniciar las clases GPS y CanBus
    gps = GPS(excel_manager)
    can = CanBus(excel_manager)

    #Iniciar el Proceso de GPS
    gps_thread = threading.Thread(target=gps.receive_gps_data)
    gps_thread.start()
    
    #Ininicar el Proceso de CanBus
    can_bus_port = "can0"
    can.connect_can_bus(can_bus_port, 'socketcan')
    can_thread = threading.Thread(target=can.receive_can_bus_data)
    can_thread.start()

    #Iniciar el Proceso de Loggeo en Excel
    excel_thread = threading.Thread(target=excel_manager.process_excel_data)
    excel_thread.start()

    