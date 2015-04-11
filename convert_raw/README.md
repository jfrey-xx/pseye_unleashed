
Depends on v4l2loopback

Load kernel module and add 3 devices:

> # modprobe v4l2loopback video_nr=10 card_label="ps eye raw"

Set framerate for loopback:

> v4l2-ctl -d /dev/video10 --set-ctrl  sustain_framerate=1
> $ v4l2loopback-ctl set-fps 30 /dev/video10

Enable raw mode and set frame rate for ps eye:

> $ v4l2-ctl -d /dev/video0 --set-ctrl focus_automatic_continuous=1 --set-parm=30

Launch pseye2loopback

> $ ./pseye2loopback


