clear
cd libWad 
make 
cd .. 
cd wadfs 
make 
cd .. 
sudo ./wadfs/wadfs -d -s sample1.wad mountdir/
