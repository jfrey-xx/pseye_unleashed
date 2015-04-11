
Depends on v4l2loopback

Load kernel module and add 3 devices:

> # modprobe v4l2loopback devices=1


Enable raw mode for ps eye:

> $ v4l2-ctl --set-ctrl focus_automatic_continuous=1
