opflag = -o myfs
flags = `pkg-config fuse --cflags --libs` -DFUSE_USE_VERSION=26 -lm -g
files = myfs.c implementation.c src/myfstree.c src/myfsbitmap.c src/myfsdisk.c

all: run

run: compile
	gdb --args ./myfs --backupfile=test.myfs ~/fuse-mnt -f

debugrun: compile
	valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f

compile:
	gcc -Wall -g $(files) $(opflag) $(flags)

checkdir:
	if [ -d "~/fuse-mnt" ]; then echo "mountpoint exists"; else mkdir ~/fuse-mnt; fi
