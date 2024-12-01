compile: wadfs.o ../libWad/Wad.o
	 g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 wadfs.o ../libWad/Wad.o -o wadfs -lfuse -L../libWad -lWad

wadfs.o: wadfs.cpp
	 g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -c wadfs.cpp

../libWad/Wad.o: ../libWad/Wad.cpp
	 g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -c ../libWad/Wad.cpp