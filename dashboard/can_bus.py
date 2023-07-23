from time import sleep
import can
import threading 
from dash_screen_data_id import DashScreenDataId
from dash_screen_controller import DashScreenController


class CanBus:

    def __init__(self):
        self.dsc = DashScreenController()


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
    
    def process_can_bus_data(self, data):
        if data[0] == 0:
            rpm = data[1] * 256 + data[2]
            self.dsc.send_screen_data(DashScreenDataId.RPM, 2000)
            wtemp = data[3]
            tps = data[4] * 256 + data[5]

            print("RPM: ", rpm, " WATERTEMP: ", wtemp, "TPS: ", tps)
        elif data[0] == 1:
            print("")  