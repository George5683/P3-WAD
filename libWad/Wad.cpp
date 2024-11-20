#include "Wad.h"
#include <fcntl.h>    // For open
#include <unistd.h>   // For read and close
#include <iostream>
using namespace std;

Wad* Wad::loadWad(const string &path){

    // Creating a new Wad object
    Wad* wad = new Wad();

    // Open the WAD file for reading
    int fd = open(path.c_str(), O_RDONLY);
    // Error Checking
    if (fd == -1) {
        cerr << "Error opening WAD file: " << path << endl;
        delete wad;
        return nullptr;
    }

    // Read the WAD file data 
    // Read the magic number [4 bytes]
    int size = 4;

    // Seek to the beginning of the file
    int offset = lseek(fd, 0, SEEK_SET);
    // Error Checking
    if (offset == -1) {
        cerr << "Error seeking to beginning of WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // Read the magic number
    int bytesRead = read(fd, &wad->magic, size);
    // Error Checking
    if (bytesRead != size) {
        cerr << "Error reading magic number from WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // Seek to the beginning of the file
    offset = lseek(fd, 4, SEEK_SET);

    // Error Checking
    if (offset == -1) {
        cerr << "Error seeking to beginning of WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // Read the Descriptor Number
    bytesRead = read(fd, &wad->DescriptorNum, size);
    // Error Checking
    if (bytesRead != size) {
        cerr << "Error reading file descriptors from WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    offset = lseek(fd, 8, SEEK_SET);
    // Error Checking
    if (offset == -1) {
        cerr << "Error seeking to beginning of WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // Read the Descriptor Offset
    bytesRead = read(fd, &wad->DescriptorOffset, size);
    // Error Checking
    if (bytesRead != size) {
        cerr << "Error reading file descriptors from WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // go to the descriptor offset
    offset = lseek(fd, wad->DescriptorOffset, SEEK_SET);
    // Error Checking
    if (offset == -1) {
        cerr << "Error seeking to beginning of WAD file: " << path << endl;
        delete wad;
        close(fd);
        return nullptr;
    }

    // Read each Descriptor and add them to the map
    for(int i = 0; i < wad->DescriptorNum; i++){
        Tree tree;
        int count = 0;
        int Element_Offset;
        int Element_Length;
        string Element_Name;

        // Read the Element Offset        
        bytesRead = read(fd, &Element_Offset, size);
        // Error Checking
        if (bytesRead != size) {
            cerr << "Error reading file descriptors from WAD file: " << path << endl;
            delete wad;
            close(fd);
            return nullptr;
        }

        // Read the Element Length
        bytesRead = read(fd, &Element_Length, size);
        // Error Checking
        if (bytesRead != size) {
            cerr << "Error reading file descriptors from WAD file: " << path << endl;
            delete wad;
            close(fd);
            return nullptr;
        }

        // Read the Name
        bytesRead = read(fd, &Element_Name, 8);
        // Error Checking
        if (bytesRead != 8) {
            cerr << "Error reading file descriptors from WAD file: " << path << endl;
            delete wad;
            close(fd);
            return nullptr;
        }

        // Checking if the file name ends with _START
        if(Element_Name.find("_START") != string::npos){
            count++;
            // Remove the _START from the name
            Element_Name = Element_Name.substr(0, Element_Name.size() - 6);
            bool Directory = true;
            // Make a TreeNode with the Name
            TreeNode TreeNode(Element_Name, Directory);

            // If the starting Directory
            if(count == 1){
                
                
            }
            else{
                // Add the TreeNode to the Tree
                tree.add(&TreeNode);
            }




        }
        else{
            
        }





        
    }






    // Close the file
    close(fd);
    return wad;
}

string Wad::getMagic(){
    return magic;
}

bool Wad::isContent(const string &path){
}

bool Wad::isDirectory(const string &path){
}

int Wad::getSize(const string &path){
    return 0;
}

int Wad::getContents(const string &path, char *buffer, int length, int offset){
}

int Wad::getDirectory(const string &path, vector<string> *directory){
}

void Wad::createDirectory(const string &path){
}

void createFile(const string &path, char *buffer, int length){
}

int writeToFile(const string &path, char *buffer, int length){
}
