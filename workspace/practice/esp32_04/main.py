from machine import Pin
import time

led = Pin(8, Pin.OUT)

print("Starting Blinky!")


while True:
    led.on()
    time.sleep(0.5)
    led.off()
    time.sleep(0.5)
