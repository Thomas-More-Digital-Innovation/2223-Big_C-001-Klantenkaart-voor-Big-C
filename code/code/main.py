import time
import network
from machine import I2C, Pin
import time
from time import sleep
from pico_i2c_lcd import I2cLcd
import urequests as requests

i2c = I2C(0, sda=Pin(0), scl=Pin(1), freq=400000)
I2C_ADDR = i2c.scan()[0]
lcd = I2cLcd(i2c, I2C_ADDR, 2, 16)
tijd = time.localtime()
tijd_format = str(tijd[2])+"/"+str(tijd[1])+" "+str(tijd[3])+":"+str(tijd[4])+":28"
print(tijd_format)
print(I2C_ADDR)

ssid = 'embedded'
password = 'IoTembedded'

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(ssid, password)

# Wait for connect or fail
max_wait = 10
while max_wait > 0:
    if wlan.status() < 0 or wlan.status() >= 3:
        break
    max_wait -= 1
    print('waiting for connection...')
    time.sleep(1)
    
# Handle connection error
if wlan.status() != 3:
    raise RuntimeError('network connection failed')
else:
    print('connected')
    status = wlan.ifconfig()
    print( 'ip = ' + status[0] )
    #post doen
    res = requests.post(url='https://the-big-c-dev.pages.dev/api/register/1234', headers={"x-api-key" : "bleepbloop"})
    print(res.text)
    #lcd tonen
    while True:
        lcd.putstr(tijd_format)
        lcd.putstr("\ngeen gratis")#\n doet nieuwe lijn
        sleep(2)
        lcd.clear()
    
    