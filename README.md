# watering-plant
- Use a capacitance soil humididity sensor
- Data is transmited to MQTT server using ESP8266 board
- An ESP8266-01 with NeoPixel subscrbes to the topic and display the color for humidity
monitor humidity content in soil by a capacitance sensor, a subscribe with an LED indicator
## Update:
- Chirp version is fun to play with but harder to put the sensor into I2C mode. I am looking to use the Analog version.

# Wiring
- Wiring diagram
<p align="center">
  <img src="images/watering_diagram.png"/>
</p>
- Reading in dry and wet soil
<p align="center">
  <img src="images/watering_humidity.png"/>
</p>

# Tutorials
- A write-up with more information is on [b-io.info](https://www.b-io.info/post/tutorial/watering-plant/)
