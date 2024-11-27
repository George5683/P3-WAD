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

    // Read each Descriptor and add them to the tree
    for (int i = 0; i < wad->DescriptorNum; i++) {

        int32_t Element_Offset;
        int32_t Element_Length;
        std::string Element_Name;

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
            cout << "Element Name: " << Element_Name << endl;
            Element_Name = Element_Name.substr(0, Element_Name.size() - 6);
            bool Directory = true;

            // Create a new TreeNode
            TreeNode* Node = new TreeNode(Element_Name, Directory, Element_Length, Element_Offset);

            if (Stack.empty()) {
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
            }
            else {
                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);
            }

            // Push the Node to the Stack
            Stack.push(Node);
        }
        // Checking if the file name starts with E#M#
        else if (Element_Name.size() >= 4 && Element_Name[0] == 'E' &&
            isdigit(Element_Name[1]) && Element_Name[2] == 'M' &&
            isdigit(Element_Name[3])) {
            // Create a TreeNode
            TreeNode* Node = new TreeNode(Element_Name, true, Element_Length, Element_Offset);
            //cout << "created node for " + Element_Name << endl;
            //cout << "Stack size is: " << Stack.size() << endl;

            if (Stack.empty()) {
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
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
                TreeNode* Node = new TreeNode(Element_Name, false, Element_Length, Element_Offset);

                // Add the Node to the Node at the top of the stack
                //cout << "Stack size is: " << Stack.size() << endl;

                Stack.top()->addChild(Node);

                // Decrease the TenCount
                TenCount--;

                if (TenCount == 0) {
                    // Pop the Node from the stack
                    Stack.pop();
                }
            }
            else {
                // Create a TreeNode
                TreeNode* Node = new TreeNode(Element_Name, false, Element_Length, Element_Offset);

                // Check if the stack is empty
                if (Stack.empty()) {
                    // add the node to the root
                    wad->tree.addChildToRoot(Node);
                }
                else {
                    // Add the Node to the Node at the top of the stack
                    Stack.top()->addChild(Node);
                }
            }
        }
    }

    return wad;
}


string Wad::getMagic(){
    return magic;
}

bool Wad::isContent(const string &path) {
    queue<string> parts;
    string part;
    stringstream ss(path);

    // Split the path by '/'
    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }

    TreeNode* currentroot = tree.getRoot();

    // For every part within parts, verify if the part is a Node in the tree before checking the child for the next part
    while (!parts.empty()) {
        // Search for the first part to see if the path is a directory
        TreeNode* temp = tree.SearchNode(parts.front(), currentroot);

        // If the node is not found, return false
        if(temp == nullptr){
            return false;
        }
        // If the node is found, check if it is a directory
        else{
            if(parts.size() == 1 && !temp->isDirectory){
                return true;
            }
            else if(!temp->isDirectory){
                return false;
            }
            else{
                currentroot = temp;
                parts.pop();
            }
        }
    }
}

bool Wad::isDirectory(const string &path){
    queue<string> parts;
    string part;
    stringstream ss(path);

    if(path == "/"){
        return true;
    }
    else if(path == ""){
        return false;
    }

    // Split the path by '/' so it only stores the name
    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }
    
    // Check if the path is only /
    if(parts.size() == 1 && parts.front() == "/"){
        return true;
    }
    else{
        // Make sure each part is a directory
        TreeNode* currentroot = tree.getRoot();

        while (!parts.empty()) {
            // Search for the first part to see if the path is a directory
            TreeNode* temp = tree.SearchNode(parts.front(), currentroot);
            
            // If the node is not found, return false
            if(temp == nullptr){
                cout << "Node is null" << endl;
                return false;
            }
            // If the node is found, check if it is a directory
            else{
                if(temp->isDirectory && parts.size() == 1){
                    return true;
                }
                else if(temp->isDirectory){
                    currentroot = temp;
                    parts.pop();
                }
                else{
                    return false;
                }
            }
        }
    }
}

int Wad::getSize(const string &path){
    // Search the tree for the path and return the size
    queue<string> parts;
    string part;
    stringstream ss(path);

    if(path == "/" || path == ""){
        return -1;
    }

    // Split the path by '/'
    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }

    TreeNode* currentroot = tree.getRoot();

    while(!parts.empty()){
    // Traverse the tree
    TreeNode* temp = tree.SearchNode(parts.front(), currentroot);

    // If the node is not found, return -1
    if(temp == nullptr){
        return -1;
    }
    // If the node is found, check if it is content
    else{
        if(parts.size() == 1 && !temp->isDirectory){
            //cout << "The size of " + parts.front() + "is: " << temp->length << endl;
            return temp->length;
        }
        else if(parts.size() == 1 && temp->isDirectory){
            return -1;

        }
        else if(temp->isDirectory){
            currentroot = temp;
            parts.pop();
        }
        else{
            return -1;
        }
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

    // Split the path by '/'
    queue<string> parts;
    string part;
    stringstream ss(path);
    string getname;

    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }

    TreeNode* currentroot = tree.getRoot();

    while(!parts.empty()){
        cout << "Part size: " << parts.size() << endl;
        
        // Traverse the tree

        getname = parts.front();
        TreeNode* temp = tree.SearchNode(getname, currentroot);
        if(temp != nullptr){
            cout << "temp name: " << temp->Name << endl;
            // print temp children
            for(TreeNode* child : temp->children){
                cout << "childs name: " << child->Name << endl;
            }
        }

        // If the node is not found, return -1
        if(temp == nullptr){
            return -1;
        }
        // If the node is found, check if it is a directory
        else{
            if(parts.size() == 1 && temp->isDirectory){
                // Add the children to the directory
                if(temp->children.size() > 0){
                    for(TreeNode* child : temp->children){
                        
                    //cout << "childs name added to directory: " << child->Name << endl;
                    // if(child->Name == "" ){
                    //     continue;
                    // }
                    directory->push_back(child->Name);
                    
                    }
                }
                
                // Return the size of directory
                cout << "Directory size: " << directory->size() << endl;
                parts.pop();
                return directory->size();
            }
            else if(temp->isDirectory){
                currentroot = temp;
                parts.pop();
            }
            else{
                return -1;
            }
        }
    }
}

int Wad::getContents(const string &path, char *buffer, int length, int offset) {
    queue<string> parts;
    string part;
    stringstream ss(path);

    if(path == ""){
        return -1;
    }

    // Split the path by '/'
    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }

    TreeNode* currentroot = tree.getRoot();

    while (!parts.empty()) {
        // Traverse the tree
        TreeNode* temp = tree.SearchNode(parts.front(), currentroot);

        // If the node is not found, return -1
        if (temp == nullptr) {
            return -1;
        }
        // If the node is found, check if it is content
        else {
            cout << endl;
            cout << "Node size is: " << temp->length << endl;
            cout << "Length is: " << length << endl;

            if(parts.size() == 1 && !temp->isDirectory){
                // Check if the offset is greater than the length no bytes get copied
                if(offset > temp->length){
                    return 0;
                }
                // Checking if the length is greater than the length of the content
                if(length > temp->length && offset > 0){
                    length = temp->length - offset;
                    cout << "Length changed to: " << length << endl;
                }
        
                // Open the WAD file for reading
                // Use the member variable fd
                // Error Checking Commented Out
                // if (fd == -1) {
                //     cerr << "Error opening WAD file: " << path << endl;
                //     close(fd);
                //     return -1;
                // }
                cout << "File Descriptor: " << fd << endl;

                // Seek the beginning of the file
                int offset2 = lseek(fd, temp->content_offset + offset, SEEK_SET);
                cout << "Offset success: " << offset2 << endl;
                // Read the content
                int bytesRead = read(fd, buffer, length);
                // Error Checking Commented Out
                // if (bytesRead != length) {
                //     cerr << "Error reading content from WAD file: " << path << endl;
                //     close(fd);
                //     return -1;
                // }
                cout << "Bytes read: " << bytesRead << endl;
                return bytesRead;
            }
            else if(parts.size() == 1 && temp->isDirectory){
                return -1;
            }
            else if(temp->isDirectory){
                currentroot = temp;
                parts.pop();
            }
        }
    }
    return -1;
}

void Wad::create32bytes(){

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


    // Write the previously read content
    while(!buffer.empty()){
        write(fd, &buffer.front(), 1);
        buffer.erase(buffer.begin());
    }
}

void Wad::createDirectory(const string &path){
    // Split the path by '/'
    queue<string> subparts;
    string part;
    stringstream ss(path);
    
    if(path == "" || path == "/"){
        return;
    }

    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            subparts.emplace(part);
        }
    }
    //cout << parts.front() << endl;
    int initialpartsize = subparts.size();
    // Getting the root
    TreeNode* currentroot = tree.getRoot();
    string name;
    
    string parentpath;

    // copy parts to a new queue
    queue<string> parts2 = subparts;

    if(parts2.size() > 1){
        // set parentpath
        while(!parts2.empty()){
            parentpath += parts2.front() + "/";
            count++;
            // check if the front is in the format E#M#
            if(parts2.front()[0] == 'E' && isdigit(parts2.front()[1]) && parts2.front()[2] == 'M' && isdigit(parts2.front()[3])){
                return;
            }
            parts2.pop();
            if(parts2.size() == 1){
                parts2.pop();
            }
        }
        cout << "Parent path: " << parentpath << endl;
    }

    // Traverse the tree
    while (!subparts.empty()) {
        cout << "parts size: " << subparts.size() << endl;
        cout << "front of parts: " << subparts.front() << endl;
        // Checking if the front of parts has a string size greater than 2
        if(subparts.front().size() > 2){
            cout << "The size of the string is greater than 2" << endl;
            return;
        }   
        name = subparts.front();
        // Search for the first part to see if the path is a directory
        TreeNode* temp = tree.SearchNode(name, currentroot);
        // If the node is not found, and its the last part
        if (temp == nullptr && initialpartsize == 1) {
            name = subparts.front();

            // Create a new directory at the root
            TreeNode* NewNode = new TreeNode(name, true, 0, 0);

            // check if Node has any children
            if(NewNode->children.size() > 0){
                for(TreeNode* child : NewNode->children){
                    cout << "New Child: " << child->Name << endl;
                }
            }

            // Add the Node to the root
            tree.addChildToRoot(NewNode);

            // pop the part
            subparts.pop();   
            cout << "first if statement" << endl;       
        }
        else if (temp == nullptr && subparts.size() == 1 && initialpartsize != 1) {
            cout << "else if statement" << endl;
            name = subparts.front();
            // Create a new directory
            TreeNode* Node = new TreeNode(name, true, 0, 0);

            // Add the Node to the current root
            currentroot->addChild(Node);

            // pop the part
            subparts.pop();
        }
        // If the node is found, check if it is a directory
        else {
            if (temp->isDirectory) {
                currentroot = temp;
                subparts.pop();
            }
            else if (!temp->isDirectory) {
                return;
            }
        }
    }

    // if the inital size is 1, add the _START and _END to the file
    cout << "Initial part size: " << initialpartsize << endl;
    if(initialpartsize == 1){
        // seeking the file to the end
        int offset = lseek(fd, (DescriptorOffset + (16*DescriptorNum)), SEEK_SET);
        if(offset == -1){
            cout << "Problem with offset for _START" << endl;
        }

        // Create a binary buffer
        std::vector<uint8_t> buffer;

        // Fill the buffer with zeros followed by the binary representation of <name>_START and <name>_END
        if (name.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (name.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_START" in binary
        buffer.insert(buffer.end(), name.begin(), name.end());
        buffer.insert(buffer.end(), {'_', 'S', 'T', 'A', 'R', 'T'});

        // Add more zeros
        if (name.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (name.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_END" in binary
        buffer.insert(buffer.end(), name.begin(), name.end());
        buffer.insert(buffer.end(), {'_', 'E', 'N', 'D'});

        // insert two more bytes of zero
        buffer.insert(buffer.end(), 2, 0x00);

        // Print the buffer for debugging
        // std::cout << "Buffer is: ";
        // for (uint8_t byte : buffer) {
        //     std::cout << std::hex << static_cast<int>(byte) << " ";
        // }
        // std::cout << std::dec << std::endl; // Switch back to decimal output

        // Write the buffer to the file
        ssize_t bytes_written = write(fd, buffer.data(), buffer.size());
        if (bytes_written == -1) {
            perror("Failed to write to file");
        }
        cout << "Bytes written: " << bytes_written << endl;
    }
    else{
        // reduce the parent path to only be GL from /GL/ or GL/ but it be AD from /GL/AD/ or GL/AD/ or AD from GL/FR/AD/
        // Remove trailing slash if it exists
        if (!parentpath.empty() && parentpath.back() == '/') {
            parentpath.pop_back();
        }

        // Find the last slash after trimming
        size_t lastSlashPos = parentpath.find_last_of('/');

        if (lastSlashPos != std::string::npos) {
            // If there's another slash before the last one, reduce further
            size_t secondLastSlashPos = parentpath.find_last_of('/', lastSlashPos - 1);
            if (secondLastSlashPos != std::string::npos) {
                parentpath = parentpath.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);
            } else {
                // Keep the part after the first slash if it exists
                parentpath = parentpath.substr(0, lastSlashPos);
            }
        }

        std::cout << "Reduced path: " << parentpath << std::endl;
        
        int val = tree.CountChildren(tree.getRoot());
        cout << "val: " << val << endl;
        tree.print();

        int start_offset = lseek(fd, -(16*(count+val)), SEEK_END);
        cout << "Start offset: " << -(16*(count+val)) << endl;

        // create the 32 bytes at the start of the parentpath directory
        create32bytes();

        // Create a binary buffer
        std::vector<uint8_t> buffer;

        // Fill the buffer with zeros followed by the binary representation of <name>_START and <name>_END
        if (name.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (name.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_START" in binary
        buffer.insert(buffer.end(), name.begin(), name.end());
        buffer.insert(buffer.end(), {'_', 'S', 'T', 'A', 'R', 'T'});

        // Add more zeros
        if (name.size() == 1) {
            buffer.insert(buffer.end(), 9, 0x00); // Add 9 zeros
        } else if (name.size() == 2) {
            buffer.insert(buffer.end(), 8, 0x00); // Add 8 zeros
        }

        // Add the name followed by "_END" in binary
        buffer.insert(buffer.end(), name.begin(), name.end());
        buffer.insert(buffer.end(), {'_', 'E', 'N', 'D'});
        // insert two more bytes of zero
        buffer.insert(buffer.end(), 2, 0x00);
        
        write(fd, buffer.data(), buffer.size());
    }
    
    // Increment the DescriptorNum by 2 on the file
    int off = lseek(fd, 4, SEEK_SET);
    DescriptorNum += 2;
    write(fd, &DescriptorNum, 4);
    

    // reset count
    count = 0;
}

void Wad::createFile(const string &path){
}

int Wad::writeToFile(const string &path, const char *buffer, int length, int offset){
    return -1;
}





