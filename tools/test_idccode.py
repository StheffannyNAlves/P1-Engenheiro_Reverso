import serial 

port = "/dev/ttyUSB0"

ser = serial.Serial(port, 115200, timeout=1)
magic = 0xAA
cmd_start_session = 0x04
arg = 0x0 
checksum = magic ^ cmd_start_session ^ arg
packet = bytes([magic, cmd_start_session, arg, checksum])

ser.write(packet)

response = ser.readline()
print("Resposta:", response.decode())

ser.close()