#include "Wad.h"
#include <fcntl.h>    // For open
#include <unistd.h>   // For read and close
#include <iostream>
#include <stack>
#include <sstream>
#include <queue>
using namespace std;

Wad* Wad::loadWad(const string &path) {

    // Creating a new Wad object
    Wad* wad = new Wad();

    // Set the path in wad
    wad->filepath = path;

    // Open the WAD file for reading 
    wad->fd = open(path.c_str(), O_RDWR);
    // Error Checking Commented Out
    // if (fd == -1) {
    //     cerr << "Error opening WAD file: " << path << endl;
    //     delete wad;
    //     return nullptr;
    // }

    // Read the WAD file data 
    // Read the magic number [4 bytes]
    int size = 4;

    // Seek to the beginning of the file
    int offset = lseek(wad->fd, 0, SEEK_SET);
    // Error Checking Commented Out
    // if (offset == -1) {
    //     cerr << "Error seeking to beginning of WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    // Read the magic number
    char magicbuffer[5] = {0}; // Allocate 5 bytes to include a null terminator
    int bytesRead = read(wad->fd, magicbuffer, size);
    // Error Checking Commented Out
    // if (bytesRead != size) {
    //     cerr << "Error reading magic number from WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    // convert the magic to be a string
    wad->magic = string(magicbuffer);

    // Seek to the beginning of the file
    offset = lseek(wad->fd, 4, SEEK_SET);

    // Error Checking Commented Out
    // if (offset == -1) {
    //     cerr << "Error seeking to beginning of WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    // Read the Descriptor Number
    bytesRead = read(wad->fd, &wad->DescriptorNum, size);
    cout << "Descriptor Number: " << wad->DescriptorNum << endl;
    // Error Checking Commented Out
    // if (bytesRead != size) {
    //     cerr << "Error reading file descriptors from WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    offset = lseek(wad->fd, 8, SEEK_SET);
    // Error Checking Commented Out
    // if (offset == -1) {
    //     cerr << "Error seeking to beginning of WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    // Read the Descriptor Offset
    bytesRead = read(wad->fd, &wad->DescriptorOffset, size);
    // Error Checking Commented Out
    // if (bytesRead != size) {
    //     cerr << "Error reading file descriptors from WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }

    // go to the descriptor offset
    offset = lseek(wad->fd, wad->DescriptorOffset, SEEK_SET);
    // Error Checking Commented Out
    // if (offset == -1) {
    //     cerr << "Error seeking to beginning of WAD file: " << path << endl;
    //     delete wad;
    //     close(fd);
    //     return nullptr;
    // }
    
    int TenCount = 0;
    // Creating a Stack
    stack<TreeNode*> Stack;
    queue<TreeNode*> q;


    // Read each Descriptor and add them to the tree
    for (int i = 0; i < wad->DescriptorNum; i++) {
        string cpath = "";

        int32_t Element_Offset;
        int32_t Element_Length;
        std::string Element_Name;
        // set the tempdiroffset to be where we are at in the file
        int tempdiroff = lseek(wad->fd, 0, SEEK_CUR);

        // Read the Element Offset
        ssize_t bytesRead = read(wad->fd, &Element_Offset, sizeof(Element_Offset));
        // Error Checking Commented Out
        // if (bytesRead != sizeof(Element_Offset)) {
        //     cerr << "Error reading file offset from WAD file: " << path << endl;
        //     delete wad;
        //     close(fd);
        //     return nullptr;
        // }

        // Read the Element Length
        bytesRead = read(wad->fd, &Element_Length, sizeof(Element_Length));
        // Error Checking Commented Out
        // if (bytesRead != sizeof(Element_Length)) {
        //     cerr << "Error reading file length from WAD file: " << path << endl;
        //     delete wad;
        //     close(fd);
        //     return nullptr;
        // }

        // Read the Name
        char nameBuffer[9] = {0}; // Allocate 9 bytes to include a null terminator
        bytesRead = read(wad->fd, nameBuffer, 8);
        // Error Checking Commented Out
        // if (bytesRead != 8) {
        //     cerr << "Error reading file descriptors from WAD file: " << path << endl;
        //     delete wad;
        //     close(fd);
        //     return nullptr;
        // }
        
        // Convert nameBuffer to a string
        Element_Name = string(nameBuffer);

        //cout << "Element Name: " << Element_Name << endl;

        // Checking if the file name ends with _START
        if (Element_Name.find("_START") != string::npos) {
            // Remove the _START from the name
            // cout << "Element Name: " << Element_Name << endl;
            Element_Name = Element_Name.substr(0, Element_Name.size() - 6);
            
            bool Directory = true;

            // Create a new TreeNode
            TreeNode* Node = new TreeNode(Element_Name, Directory, Element_Length, Element_Offset, tempdiroff);

            if (Stack.empty()) {
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
                // set cpath
                cpath = Node->Name;
            }
            else {
                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);
                
            }

            // insert into the map
            

            // Push the Node to the Stack
            Stack.push(Node);

        }
        // Checking if the file name starts with E#M#
        else if (Element_Name.size() >= 4 && Element_Name[0] == 'E' &&
            isdigit(Element_Name[1]) && Element_Name[2] == 'M' &&
            isdigit(Element_Name[3])) {

            // Create a TreeNode
            TreeNode* Node = new TreeNode(Element_Name, true, Element_Length, Element_Offset, tempdiroff);
            //cout << "created node for " + Element_Name << endl;
            //cout << "Stack size is: " << Stack.size() << endl;

            if (Stack.empty()) {
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
                // set cpath
                cpath = Node->Name;
                //cout << "added node to root" << endl;
            }
            else {
                // Add the Node to the children of Node at the top of the stack
                Stack.top()->addChild(Node);
                
            }

            // Push the Node to the Stack
            Stack.push(Node);

            
            //cout << "Stack size after E#M# is: " << Stack.size() << endl;

            TenCount = 10;

        }
        // Checking if the name ends with _END
        else if (Element_Name.find("_END") != string::npos) {
            // Pop the Node from the stack
            Stack.pop();
        }
        // If just a normal file
        else {
            // Check if the TenCount is set
            if (TenCount != 0) {
                //cout << "TenCount is: " << TenCount << endl;
                // Create a TreeNode
                TreeNode* Node = new TreeNode(Element_Name, false, Element_Length, Element_Offset , tempdiroff);

                // Add the Node to the Node at the top of the stack
                //cout << "Stack size is: " << Stack.size() << endl;

                Stack.top()->addChild(Node);

                

                // insert into the map
                

                // Decrease the TenCount
                TenCount--;

                if (TenCount == 0) {
                    // Pop the Node from the stack
                    Stack.pop();
                }
            }
            else {
                // Create a TreeNode
                TreeNode* Node = new TreeNode(Element_Name, false, Element_Length, Element_Offset, tempdiroff);

                // Check if the stack is empty
                if (Stack.empty()) {
                    // add the node to the root
                    wad->tree.addChildToRoot(Node);

                    // set cpath
                    cpath = Node->Name;
                }
                else {
                    // Add the Node to the Node at the top of the stack
                    Stack.top()->addChild(Node);

                }

                // insert into the map
                
            }
        }
    }
    // print the whole tree
    wad->tree.print();
    cout << endl;

    // Build the map from the tree
    wad->tree.BuildMap();

    // print the map
    // wad->tree.PrintMap();
    return wad;
}


string Wad::getMagic(){
    return magic;
}

bool Wad::isContent(const string &path) {
    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // Get the node from the map
    TreeNode* node = tree.SearchMap(path2);

    // If the node is not found, return false
    if (node == nullptr) {
        return false;
    }
    // If the node is found, check if it is content
    else {
        return !node->isDirectory;
    }
}

bool Wad::isDirectory(const string &path){
    // checking if the path is the root or nothing
    if(path == "/"){
        return true;
    }
    else if(path == ""){
        return false;
    }

    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // Get the node from the map
    TreeNode* node = tree.SearchMap(path2);

    // If the node is not found, return false
    if (node == nullptr) {
        return false;
    }
    // If the node is found, check if it is content
    else {
        return node->isDirectory;
    }
    
}

int Wad::getSize(const string &path){
    // checking if the path is only / or nothing
    if(path == "/" || path == ""){
        return -1;
    }
    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // Get the node from the map
    TreeNode* node = tree.SearchMap(path2);

    // If the node is not found, return -1
    if (node == nullptr) {
        return -1;
    }
    // If the node is found, check if it is content
    else {
        if (node->isDirectory) {
            return -1;
        }
        else {
            return node->length;
        }
    }
}

int Wad::getDirectory(const string &path, vector<string> *directory){
    // checking if the path is only /
    if(path == "/"){
        // Get the root node
        TreeNode* currentNode = tree.getRoot();

        // Add the children to the directory
        for(auto child : currentNode->children){
            directory->push_back(child->Name);
        }

        // Return the number of children
        return currentNode->children.size();
    }
    else if(path == ""){
        return -1;
    }

    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // cout << "Path2: " << path2 << endl;
    // Get the node from the map
    TreeNode* node = tree.SearchMap(path2);
    // cout << "Node: " << node->Name << endl;

    // If the node is not found, return -1
    if (node == nullptr) {
        return -1;
    }
    // If the node is found, check if it is a directory
    else {
        if (!node->isDirectory) {
            return -1;
        }
        else {
            // Add the children to the directory
            for(auto child : node->children){
                directory->push_back(child->Name);
            }

            // Return the number of children
            return node->children.size();
        }
    }
}

int Wad::getContents(const string &path, char *buffer, int length, int offset) {
    // Checking if the path is empty
    if(path == ""){
        return -1;
    }

    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // Get the node from the map
    TreeNode* node = tree.SearchMap(path2);

    // If the node is not found, return -1
    if (node == nullptr) {
        return -1;
    }
    // If the node is found, check if it is content
    else {
        if (node->isDirectory) {
            return -1;
        }
        else {
            // Check if the offset is greater than the length no bytes get copied
            if(offset > node->length){
                return 0;
            }
            // Checking if the length is greater than the length of the content
            if(length > node->length && offset > 0){
                length = node->length - offset;
                cout << "Length changed to: " << length << endl;
            }

            // Seek to the offset
            lseek(fd, node->content_offset + offset, SEEK_SET);

            // Read the content
            int bytesRead = read(fd, buffer, length);

            // Return the number of bytes read
            return bytesRead;
        }
    }
}

void Wad::create32bytes(int diroff){

    // Read the content from the offset to the end of the file
    std::vector<char> buffer;
    char temp;
    while (read(fd, &temp, 1) == 1) {
        buffer.push_back(temp);
    }
    

    // Write 32 bytes of zeros in binary
    for (int i = 0; i < 32; i++) {
        write(fd, "\0", 1);
    }

    // go back to the diroff
    lseek(fd, diroff + 32, SEEK_SET);


    // Write the previously read content
    while(!buffer.empty()){
        write(fd, &buffer.front(), 1);
        buffer.erase(buffer.begin());
    }
    
}

void Wad::UpdateFileDescriptorOffsets(){
    // get to the descriptor location
    lseek(fd, DescriptorOffset, SEEK_SET);

    for (int i = 0; i < DescriptorNum; i++) {
        int32_t New_offset;
        int32_t New_Length;
        string New_Name;

        int t = lseek(fd, 0, SEEK_CUR);

        ssize_t bytesRead = read(fd, &New_offset, sizeof(New_offset));

        bytesRead = read(fd, &New_Length, sizeof(New_Length));

        char nameBuffer [9] = {0};

        bytesRead = read(fd, nameBuffer, 8);

        // convert the nameBuffer to a string
        New_Name = string(nameBuffer);

        // Remove _START or _END from the name
        if (New_Name.find("_START") != string::npos) {
            New_Name = New_Name.substr(0, New_Name.size() - 6);
            
        }
        else if (New_Name.find("_END") != string::npos) {
            New_Name = New_Name.substr(0, New_Name.size() - 4);
        }

        



        
        
    }
}

void Wad::createDirectory(const string &path){
    if(path == "" || path == "/"){
        return;
    }

    // Check if the path has a E followed by a number M followed by a number within the string
    if (path.find("E") != string::npos && isdigit(path[path.find("E") + 1]) &&
        path.find("M") != string::npos && isdigit(path[path.find("M") + 1])) {
        cout << "Path contains E#M# pattern" << endl;
        return;
    }

    // Remove the first / in the path if it is there
    string path2 = path;
    if (path2[0] == '/') {
        path2 = path2.substr(1);
    }

    // Remove the last / in the path if it is there
    if (path2[path2.size() - 1] == '/') {
        path2 = path2.substr(0, path2.size() - 1);
    }

    // check if the path doesnt has a / in it, we are going to the root
    if(path2.find("/") == string::npos){
        
        string newName = path2;

        // create the node for the new directory
        TreeNode* Node = new TreeNode(newName, true, 0, 0, 0);

        // add the node to the root
        tree.addChildToRoot(Node);

        // insert into the map
        tree.InsertIntoMap(path2, Node);
        cout << "Inserted into map:" << path2 << endl;

        // seeking the file to the end
        int offset = lseek(fd, (DescriptorOffset + (16*DescriptorNum)), SEEK_SET);
        if(offset == -1){
            cout << "Problem with offset for _START" << endl;
        }

        // Create a binary buffer
        std::vector<uint8_t> buffer;

        // Fill the buffer with zeros followed by the binary representation of <name>_START and <name>_END
        if (newName.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (newName.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_START" in binary
        buffer.insert(buffer.end(), newName.begin(), newName.end());
        buffer.insert(buffer.end(), {'_', 'S', 'T', 'A', 'R', 'T'});

        // Add more zeros
        if (newName.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (newName.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_END" in binary
        buffer.insert(buffer.end(), newName.begin(), newName.end());
        buffer.insert(buffer.end(), {'_', 'E', 'N', 'D'});

        // insert two more bytes of zero
        buffer.insert(buffer.end(), 2, 0x00);

        // write the buffer to the file
        ssize_t bytes_written = write(fd, buffer.data(), buffer.size());
        if (bytes_written == -1) {
            perror("Failed to write to file");
        }
        cout << "Bytes written: " << bytes_written << endl;

    }
    else{
        // Check if the path minus the last name is a directory
        string parentpath = path2.substr(0, path2.find_last_of("/"));
        if (!isDirectory(parentpath)) {
            return;
        }
        
        // Get the name of the new directory
        string newName = path2.substr(path2.find_last_of("/") + 1);

        // get the node for the parent from the map
        TreeNode* parentnode = tree.SearchMap(parentpath);
        cout << "Parent Node: " << parentnode->Name << endl;

        // create the node for the new directory
        TreeNode* Node = new TreeNode(newName, true, 0, 0, 0);

        // add the node to the parent children
        parentnode->addChild(Node);

        // insert into the map
        tree.InsertIntoMap(path2, Node);
        cout << "Inserted into map:" << path2 << endl;

        int contchildrennum = tree.countContent(parentnode);
        int dirchildrennum = tree.countDirectories(parentnode);

        int tempoff = (parentnode->Diroffset + DescriptorOffset) + (16* contchildrennum) + (16* dirchildrennum);

        // seek to the parent node's content offset
        lseek(fd, tempoff, SEEK_SET);

        // create the 32 bytes
        create32bytes(tempoff);

        lseek(fd, tempoff, SEEK_SET);

        // Create a binary buffer
        std::vector<uint8_t> buffer;

        // Fill the buffer with zeros followed by the binary representation of <name>_START and <name>_END
        if (newName.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (newName.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_START" in binary
        buffer.insert(buffer.end(), newName.begin(), newName.end());
        buffer.insert(buffer.end(), {'_', 'S', 'T', 'A', 'R', 'T'});
        buffer.insert(buffer.end(), 0x00);

        // Add more zeros
        if (newName.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (newName.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_END" in binary
        buffer.insert(buffer.end(), newName.begin(), newName.end());
        buffer.insert(buffer.end(), {'_', 'E', 'N', 'D'});
        buffer.insert(buffer.end(), 0x00);

        // insert two more bytes of zero
        buffer.insert(buffer.end(), 2, 0x00);

        // write the buffer to the file
        ssize_t bytes_written = write(fd, buffer.data(), buffer.size());
        if (bytes_written == -1) {
            perror("Failed to write to file");
        }

    }

    UpdateFileDescriptorOffsets();
    
    // Increment the DescriptorNum by 2 on the file
    int off = lseek(fd, 4, SEEK_SET);
    DescriptorNum += 2;
    write(fd, &DescriptorNum, 4);
}

void Wad::createFile(const string &path){
}

int Wad::writeToFile(const string &path, const char *buffer, int length, int offset){
    return -1;
}







