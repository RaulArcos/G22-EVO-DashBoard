from enum import Enum

class WorkingModeId(Enum):
    NO_DATA = 0
    ONLY_CAN = 1
    ONLY_GPS = 2
    CANGPS = 3