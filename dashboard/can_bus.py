from time import sleep
import can


class CanBus:

    def connect_can_bus(self, channel, bustype):
        try:
            self.bus_ = can.interface.Bus(channel, bustype)
            print("Can Bus Conectado!")
        except OSError as e:
            print(e)
            print("No encontré el CAN_BUS, reintentando...")
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
                    print("Message: ", decimal_array)
        except KeyboardInterrupt:
            self.bus_.shutdown()