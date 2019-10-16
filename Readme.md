fabpide2 is a PCB kit that breaks out Stand-alone USB PD sink controller
[STUSB4500](https://www.st.com/en/interfaces-and-transceivers/stusb4500.html)
by STMicroelectronics.

You can [buy this kit from
tindie](https://www.tindie.com/products/18263/).

See [my blog
post](https://blog.oxplot.com/usb-pd-standalone-sink-controller/) for a
bit of background.

# Flash configuration

You need the [official GUI
app](https://github.com/usb-c/STUSB4500/tree/master/GUI) (copy under
vendor directory here as well) to generate
configuration values which can then be flashed using an Arduino via I2C
bus. An arduino sketch is provided here for this purpose. See the
instructions in the header comment of the sketch.

The use of GUI is a temporary measure while I work out how each register
is encoded so as to eliminate the need for this windows GUI application.

# Is this open source?

Not yet as I need to clean things up quite a bit. Once done, I will
include all the design files in KiCad as well as ready to manufacture
gerber files.

<a href="https://www.tindie.com/stores/oxplot/?ref=offsite_badges&utm_source=sellers_oxplot&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>
