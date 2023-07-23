import openpyxl
from working_mode_id import WorkingModeId

class ExcelManager:
    
    #Se declaran las variables que se van a utilizar
    def __init__(self, excel_file_path):
            self.excel_file_path = excel_file_path
            self.latitude = None
            self.longitude = None
            self.timestamp_gps = None
            self.speed = None
            self.rpm = None
            self.wtemp = None
            self.battvolt = None
            self.tps = None
            self.timestamp_can = None
    
    #Comprueba que los datos de GPS y CAN esten sincronizados (Máximo un segundo de diferencia)
    def gps_can_are_sync(self):
        difference = abs(self.timestamp_gps - self.timestamp_can)
        return difference < 250
    
    #Comprueba que los datos de CAN estén disponibles
    def can_data_avaitable(self):
        if self.rpm is not None and self.wtemp is not None and self.battvolt is not None and self.tps is not None and self.timestamp_can is not None:
            return True 
        else:
            return False
    

    #Comprueba que los datos de GPS estén disponibles
    def gps_data_avaitable(self):
        if self.latitude is not None and self.longitude is not None and self.speed is not None and self.timestamp_gps is not None:
            return True 
        else:
            return False

    #Crea el archivo de excel
    def create_excel_file(self):
        wb = openpyxl.Workbook()
        sheet = wb.active
        sheet.append(["TimeStamp", "Latitude", "Longitude", "Speed (km/h)", "RPM", "Water Temp", "battvolt", "TPS"])
        wb.save(self.excel_file_path)

    #Guarda los datos de GPS
    def save_gps_data(self, latitude, longitude, timestamp, speed):
        self.latitude = latitude
        self.longitude = longitude
        self.timestamp_gps = timestamp
        self.speed = speed

    #Guarda los datos de CAN
    def save_can_data(self, rpm, wtemp, battvolt, tps, timestamp):
        self.rpm = rpm
        self.wtemp = wtemp
        self.battvolt = battvolt
        self.tps = tps
        self.timestamp_can = timestamp
    
    #Procesa los datos de excel
    def process_excel_data(self):
        while True:
            working_mode = WorkingModeId.NO_DATA
            if self.can_data_avaitable() and self.gps_data_avaitable() and self.gps_can_are_sync():
                working_mode = WorkingModeId.CANGPS
            elif not self.can_data_avaitable() and self.gps_data_avaitable():
                working_mode = WorkingModeId.ONLY_GPS
            elif self.can_data_avaitable() and not self.gps_data_avaitable():
                working_mode = WorkingModeId.ONLY_CAN
            
            if working_mode != WorkingModeId.NO_DATA:
                self.save_data_to_excel(working_mode)

    #Guarda los datos en el archivo de excel y resetea los datos guardados anteriormente
    def save_data_to_excel(self, working_mode):
        wb = openpyxl.load_workbook(self.excel_file_path)
        sheet = wb.active
        if(working_mode == WorkingModeId.CANGPS):
            sheet.append([self.timestamp_gps, self.latitude, self.longitude, self.speed, self.rpm, self.wtemp, self.battvolt, self.tps])
        elif(working_mode == WorkingModeId.ONLY_CAN):
            sheet.append([self.timestamp_can, "N/A", "N/A", "N/A", self.rpm, self.wtemp, self.battvolt, self.tps])
        elif(working_mode == WorkingModeId.ONLY_GPS):
            sheet.append([self.timestamp_gps, self.latitude, self.longitude, self.speed, "N/A", "N/A", "N/A", "N/A"])
        
        self.latitude = None
        self.longitude = None
        self.timestamp_gps = None
        self.speed = None
        self.rpm = None
        self.wtemp = None
        self.battvolt = None
        self.tps = None
        self.timestamp_can = None

        wb.save(self.excel_file_path)

    