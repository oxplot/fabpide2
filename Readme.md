**[See the newer fpx module](https://github.com/oxplot/fpx)**

fabpide2 is a PCB kit that breaks out Stand-alone USB PD sink controller
[STUSB4500](https://www.st.com/en/interfaces-and-transceivers/stusb4500.html)
by STMicroelectronics.

You can [buy this kit from
tindie](https://www.tindie.com/products/18263/).

See [my blog
post](https://blog.oxplot.com/usb-pd-standalone-sink-controller/) for a
bit of background.

# Flashing configuration with Arduino

One way to use STUSB4500 is to flash desired PDO configurations in its
NVM and it will work autonomously from then on. To use the chip in this
manner:

1. Open the [arduino
   flasher](arduino/stusb4500_flasher/stusb4500_flasher.ino) in Arduino
   IDE.

2. Install the [Software I2C library in this
   repo](arduino/libs/SoftI2CMaster.zip) on your Arduino IDE (via Sketch
   menu -> Include Library -> Add .ZIP library) — *note: adding the
   library may automatically add `#include` lines at the top of the sketch -
   be sure to remove those*

3. Follow the comments on the top of the sketch.

# Control via uC

Another way to use STUSB4500 is to connect it to an external uC to
dynamically control the chip and respond to events.

[Andrew](https://github.com/ardnew) has implemented an [Arduino
library](https://github.com/ardnew/STUSB4500)
called `STUSB4500` which is available in the official Arduino IDE
repository and can be installed from inside the IDE.

# Is this open source?

Yes. You can find the gerber files for revision 2 under `gerber/`
directory.

<a href="https://www.tindie.com/stores/oxplot/?ref=offsite_badges&utm_source=sellers_oxplot&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>
