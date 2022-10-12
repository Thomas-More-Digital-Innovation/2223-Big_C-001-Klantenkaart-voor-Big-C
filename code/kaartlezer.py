import board
import busio
from digitalio import DigitalInOut
from adafruit_pn532.spi import PN532_SPI

# SPI connection:
spi = busio.SPI(clock=board.GP2, MOSI=board.GP3, MISO=board.GP4)
cs_pin = DigitalInOut(board.GP5)
pn532 = PN532_SPI(spi, cs_pin, debug=False)

ic, ver, rev, support = pn532.firmware_version
print("Found PN532 with firmware version: {0}.{1}".format(ver, rev))

# Configure PN532 to communicate with MiFare cards
pn532.SAM_configuration()

lastCardUID = 0

print("Waiting for RFID/NFC card...")
while True:
    # Check if a card is available to read
    uid = pn532.read_passive_target(timeout=0.5)
    #print(".", end="")
    # Try again if no card is available.
    if uid is None:
        continue
    cardUID = [hex(i) for i in uid]#binairy naar hexadecimaal
    if cardUID != lastCardUID:
        #print(uid)
        print(cardUID)
        print(int.from_bytes(uid,"big"))#maakt van de binairy array een decimaal nummer
        print()
        
        lastCardUID = cardUID
