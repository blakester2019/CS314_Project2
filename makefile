all:
	gcc -Wall fs.c filefs.c `pkg-config fuse --cflags --libs` -o filefs