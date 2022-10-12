from machine import I2C, Pin
import time
from time import sleep
from pico_i2c_lcd import I2cLcd
i2c = I2C(0, sda=Pin(0), scl=Pin(1), freq=400000)

I2C_ADDR = i2c.scan()[0]
lcd = I2cLcd(i2c, I2C_ADDR, 2, 16)
tijd = time.localtime()
tijd_format = str(tijd[2])+"/"+str(tijd[1])+" "+str(tijd[3])+":"+str(tijd[4])+":28"
print(tijd_format)
print(I2C_ADDR)
while True:
    lcd.putstr(tijd_format)
    lcd.putstr("\ngeen gratis")#\n doet nieuwe lijn
    sleep(2)
    lcd.clear()
    #lcd.backlight_on()   
    #lcd.backlight_off()
    #lcd.hide_cursor()

    