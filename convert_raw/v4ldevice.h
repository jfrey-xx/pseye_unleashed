

 void open_device(char* devicename); 
 void close_device(void);
 void init_device (int width, int height );
 void init_userp (unsigned int buffer_size);
 void init_mmap (void);
 void init_read (unsigned int buffer_size);
 void uninit_device (void);
 void start_capturing (void);
 void stop_capturing (void);
 int read_frame (void);
 void errno_exit (const char *s);
 int xioctl (int fh, int request, void *arg);
 
 unsigned char* snapFrame();
