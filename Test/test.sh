clear
cd ../libWad
make
cd ../Test
g++ -o wad_dump wad_dump.cpp -I../libWad -L../libWad -lWad
./wad_dump sample1.wad
# ...existing code...