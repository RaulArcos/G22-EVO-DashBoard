import serial

class DashScreenController:

    #Inicamos las variables de la clase
    def __init__(self):
        self.send_data_template = bytearray([0x5A, 0xA5, 0x05, 0x82])
        self.port = '/dev/ttyS3'
        self.ser = serial.Serial(self.port, 9600, timeout=1)

    #Funcion para enviar datos a la pantalla, cada vez que se quiera enviar un dato se debe llamar a esta funcion
    def send_screen_data(self, DataId, data):
        send_data = self.send_data_template.copy()
        send_data += DataId.value.to_bytes(1, byteorder='big')
        if type(data).__name__ == "int":
            send_data += (0x00).to_bytes(1, byteorder='big')
            byte = data.to_bytes(2, byteorder='big')
            send_data += byte
            self.ser.write(send_data)
            




    