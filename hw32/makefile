username = $(shell whoami)
SRCDIR = src
files = myfs.c implementation.c $(SRCDIR)/fstree.c $(SRCDIR)/bitmap.c $(SRCDIR)/fsdisk.c
opflag = -o myfs
flags = `pkg-config fuse --cflags --libs` -DFUSE_USE_VERSION=26 -lm -g

all: run

run: compile
	gdb --args ./myfs --backupfile=test.myfs /home/$(username)/Desktop/mountpoint -f

debugrun: dcompile
	valgrind --track-origins=yes ./fs -d -f -s /home/$(username)/Desktop/mountpoint

compile: checkdir
	gcc -Wall -g $(files) $(opflag) $(flags)

dcompile: checkdir	
	gcc -Wall -g $(files) $(opflag) $(flags)

checkdir:
	if [ -d "/home/$(username)/Desktop/mountpoint" ]; then echo "mountpoint exists"; else mkdir /home/$(username)/Desktop/mountpoint; fi
