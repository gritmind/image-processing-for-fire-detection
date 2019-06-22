CC = g++

SRCS = client.c
PROG = client

OPENCV = -lopencv_imgproc -lopencv_core -lopencv_highgui -L/usr/lib/uv4l/uv4lext/armv6l -luv4lext -Wl,-rpath,'/usr/lib/uv4l/uv4lext/armv6l'

$(PROG):$(SRCS)
	$(CC) $(SRCS) -o $(PROG) $(OPENCV)

clean: 
	rm -rf $(PROG)

