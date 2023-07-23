import board
import neopixel
import time

# Configure the LED strip
LED_COUNT = 144    # Change this to the number of LEDs in your strip
LED_PIN = board.MOSI   # GPIO pin connected to the LED strip
LED_BRIGHTNESS = 0.2  # Set the brightness (0.0 to 1.0)
ORDER = neopixel.RGB  # RGB or GRB, depending on the LED strip configuration

pixels = neopixel.NeoPixel(LED_PIN, LED_COUNT, brightness=LED_BRIGHTNESS, auto_write=False, pixel_order=ORDER)

def set_led_color(led_index, color):
    pixels[led_index] = color
    pixels.show()

def clear_leds():
    pixels.fill((0, 0, 0))
    pixels.show()

if __name__ == "__main__":
    try:
        # Example: Set all LEDs to orange
        clear_leds()
        for i in range(LED_COUNT):
            set_led_color(i, (255, 165, 0))  # RGB color for orange
            time.sleep(0.1)

    except KeyboardInterrupt:
        clear_leds()