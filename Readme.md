fabpide2 is a PCB kit that breaks out Stand-alone USB PD sink controller
[STUSB4500](https://www.st.com/en/interfaces-and-transceivers/stusb4500.html)
by STMicroelectronics.

You can [buy this kit from
tindie](https://www.tindie.com/products/18263/).

See [my blog
post](https://blog.oxplot.com/usb-pd-standalone-sink-controller/) for a
bit of background.

# Flashing configuration with Arduino

* Install the [Software I2C library in this
  repo](arduino/libs/SoftI2CMaster.zip) on your Arduino IDE (via Sketch
  menu -> Include Library -> Add .ZIP library)

* Open the [arduino
  flasher](arduino/stusb4500_flasher/stusb4500_flasher.ino) ans follow
  the comments on the top of the file.

# Other drivers

STUSB4500 can also be controlled dynamically with an external uC.
[Andrew](https://github.com/ardnew) has implemented an STM32 HAL driver
for the chip which is available at https://github.com/ardnew/upd-touch

# Is this open source?

Yes. You can find the gerber files for revision 2 under `gerber/`
directory.

<a href="https://www.tindie.com/stores/oxplot/?ref=offsite_badges&utm_source=sellers_oxplot&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>
