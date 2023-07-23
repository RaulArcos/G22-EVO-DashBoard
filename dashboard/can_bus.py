from time import sleep
import can
import threading 
from dash_screen_data_id import DashScreenDataId
from dash_screen_controller import DashScreenController
import time
import math


class CanBus:

    #Se declaran las variables que se van a utilizar
    def __init__(self, excel_manager):
        self.dsc = DashScreenController()
        self.excel_manager = excel_manager

    #Se conecta al CAN_BUS
    def connect_can_bus(self, channel, bustype):
        try:
            self.bus_ = can.interface.Bus(channel, bustype)
            print("Can Bus Conectado!")
        except OSError as e:
            print(e)
            print("No encontré el CAN_BUS, reintentando...")
            self.dsc.send_screen_data(DashScreenDataId.RPM, 2000)
            sleep(1)
            self.connect_can_bus(channel, bustype)
    
    #Recibe los datos del CAN_BUS
    def receive_can_bus_data(self):
        try:
            while True:
                decimal_array = []
                message = self.bus_.recv(timeout=1)
                if message is not None:
                    for byte in message.data:
                        decimal_array.append(byte)
                    process_can_bus_thread = threading.Thread(target=self.process_can_bus_data, args=(decimal_array,))
                    process_can_bus_thread.start()
        except KeyboardInterrupt:
            self.bus_.shutdown()
    
    #Procesa los datos del CAN_BUS y los envia tanto a la pantalla como al log
    def process_can_bus_data(self, data):
        if data[0] == 1:
            rpm = data[1] * 256 + data[2]
            self.dsc.send_screen_data(DashScreenDataId.RPM, rpm)
            wtemp = int((data[3] - 32) * (5/9)) # Paso de Fº a Cº
            self.dsc.send_screen_data(DashScreenDataId.WTEMP, wtemp)
            battvolt = float((data[4] * 256 + data[5])/100)
            self.dsc.send_screen_data(DashScreenDataId.GEAR, battvolt)
            tps = data[6] * 256 + data[7]
            self.dsc.send_screen_data(DashScreenDataId.TPS, tps)

            timestamp = int(time.perf_counter() * 1000)
            self.excel_manager.save_can_data(rpm, wtemp, battvolt, tps, timestamp)

            print("CanBus::process_can_bus_data - RPM: ", rpm, " WATERTEMP: ", wtemp, "BATTVOLT: ", battvolt, "TPS: ", tps)
        elif data[0] == 2:  
            iat = data[1]