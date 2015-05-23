
# compile module

Install kernel sources in =/usr/src/linux=, copy over ov534.c to =drivers/media/usb/gspca/=

> apt-get install kernel-package build-essential linux-source
> cd /usr/src
> tar xvjf linux-source-3.XX.YY.tar.bz2
> ln -s usr/src/linux-source-3.XX.YY /usr/src/linux
> cp /usr/src/linux-headers-$(uname -r)/Module.symvers /usr/src/linux
> make oldconfig
> make modules_prepare
> make SUBDIRS=drivers/media/usb/gspca/ modules 
> cp drivers/media/usb/gspca/gspca_ov534.ko /lib/modules/$(uname -r)/kernel/drivers/media/usb/gspca/
> rmmod gspca_ov534
> modprobe gspca_ov534

# launch program

Depends on v4l2loopback + libopencv-dev + libv4l-dev

## V4L2 loopback

Load kernel module and add 3 devices:

> # modprobe v4l2loopback video_nr=10 card_label="ps eye raw"

Set framerate for loopback:

> $ v4l2-ctl -d /dev/video10 --set-ctrl  sustain_framerate=1

> # v4l2loopback-ctl set-fps 30 /dev/video10

## PSeye

Enable raw mode and set frame rate for ps eye:

> $ v4l2-ctl -d /dev/video0 --set-ctrl focus_automatic_continuous=1 --set-parm=30

Disable set exposure to manual and then to higher value, disable autogain

## Launch pseye2loopback

> $ ./pseye2loopback --width 640 --height 480 --video-in /dev/video0 --video-out /dev/video10

WARNING: due to the way the bayer matrix is encoded / decoded, **do not** use the V4L "vertical flip" flag. Instead use arguments for pseye2loopback "--vflip" (also "--hflip" added for commodity).
