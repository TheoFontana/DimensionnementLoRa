import serial
import time

arduino_port = '/dev/cu.usbmodem11301' #serial port of Arduino
baud = 115200 #arduino runs at 115200 baud
fileName='data.csv' #name of the CSV file generated

ser = serial.Serial(arduino_port, baud)

print('\nConnected to Arduino port:' + arduino_port)
print('\nThe folowing data will be added to ' + fileName + '\n')

while True :
    time.sleep(5)
    getData=str(ser.readline())
    data=getData[2:][:-5]
    print(data)

    # Add the data to the file
    file = open(fileName, 'a')
    file.write(data + '\n') 
    file.close()