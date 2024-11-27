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
    wad->path = path;

    // Open the WAD file for reading
    wad->fd = open(path.c_str(), O_RDONLY);
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
    stringstream
    ss(path);

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
        // If the node is found, check if it is a directory
        else{
            if(parts.size() == 1 && temp->isDirectory){
                // Add the children to the directory
                for(auto child : temp->children){
                    //cout << "childs name added to directory: " << child->Name << endl;
                    directory->push_back(child->Name);
                }

                // Return the number of children
                return temp->children.size();
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

void Wad::create16bytes(int offset){
    // Seek to the offset
    int lseek_offset = lseek(fd, offset, SEEK_SET);
    if (lseek_offset == -1) {
        perror("lseek");
        return;
    }

    // Read the content from the offset to the end of the file
    std::vector<char> buffer;
    char temp;
    while (read(fd, &temp, 1) == 1) {
        buffer.push_back(temp);
    }

    // Move the file pointer to the new position (offset + 16)
    lseek_offset = lseek(fd, offset + 16, SEEK_SET);
    if (lseek_offset == -1) {
        perror("lseek");
        return;
    }

    // Write the previously read content
    if (!buffer.empty()) {
        write(fd, buffer.data(), buffer.size());
    }

    // Move the file pointer back to the original offset
    lseek_offset = lseek(fd, offset, SEEK_SET);
    if (lseek_offset == -1) {
        perror("lseek");
        return;
    }

    // Write 16 bytes
    std::string sixteen = "0000000000000000";
    write(fd, sixteen.c_str(), sixteen.size());
}

void Wad::createDirectory(const string &path){
    // Split the path by '/'
    queue<string> parts;
    string part;
    stringstream
    ss(path);
    
    if(path == "" || path == "/"){
        return;
    }

    while (getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.emplace(part);
        }
    }
    //cout << parts.front() << endl;
    int initialpartsize = parts.size();
    // Getting the root
    TreeNode* currentroot = tree.getRoot();
    string name;
    
    string parentpath;

    // copy parts to a new queue
    queue<string> parts2 = parts;

    int count = 0;
    if(parts.size() > 1){
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
    }

    // Traverse the tree
    while (!parts.empty()) {
        cout << "front of parts: " << parts.front() << endl;
        // Checking if the front of parts has a string size greater than 2
        if(parts.front().size() > 2){
            cout << "The size of the string is greater than 2" << endl;
            return;
        }
        // Search for the first part to see if the path is a directory
        TreeNode* temp = tree.SearchNode(parts.front(), currentroot);
        // If the node is not found, and its the last part
        if (temp == nullptr && initialpartsize == 1) {
            name = parts.front();

            // Create a new directory at the root
            TreeNode* Node = new TreeNode(parts.front(), true, 0, 0);

            // Add the Node to the root
            tree.addChildToRoot(Node);

            // pop the part
            parts.pop();            
        }
        else if (temp == nullptr && parts.size() == 1) {
            name = parts.front();
            // Create a new directory
            TreeNode* Node = new TreeNode(parts.front(), true, 0, 0);

            // Add the Node to the current root
            currentroot->addChild(Node);

            // pop the part
            parts.pop();
        }
        // If the node is found, check if it is a directory
        else {
            if (!temp->isDirectory) {
                return;
            }
            else if (temp->isDirectory) {
                currentroot = temp;
                parts.pop();
            }
        }
    }

    // if the inital size is 1, add the _START and _END to the root
    if(initialpartsize == 1){
        // create the bytes at the end of the file
        create16bytes(DescriptorOffset + DescriptorNum * 16);
        create16bytes(DescriptorOffset + DescriptorNum * 16 + 16);

        // seeking the file to the end
        int offset = lseek(fd, DescriptorOffset + DescriptorNum*16, SEEK_SET);

        // create a buffer with the first few bytes a zero followed by the <name>_START
        string buffer;
        if(name.size() == 1){
            buffer = "000000000" + name + "_START"
            + "00000000000" + name + "_END";
        }
        else if(name.size() == 2){
            buffer = "00000000" + name + "_START"
            + "0000000000" + name + "_END";
        }
        
        write(fd, buffer.c_str(), buffer.size());
    }
    else{
        // search the tree for the path
        int start_offset = lseek(fd, -(16*count), SEEK_END);
        int end_offset = lseek(fd, -(16*(count-1)), SEEK_END);

        // create the 16 bytes at the start of the parentpath directory
        create16bytes(start_offset);
        create16bytes(end_offset);

        // seeking the file to the end
        int offset = lseek(fd, -(16*count), SEEK_END);

        // create a buffer with the first few bytes a zero followed by the <name>_START
        string buffer;
        if(name.size() == 1){
            buffer = "000000000" + name + "_START"
            + "00000000000" + name + "_END";
        }
        else if(name.size() == 2){
            buffer = "00000000" + name + "_START"
            + "0000000000" + name + "_END";
        }
        
        write(fd, buffer.c_str(), buffer.size());
    }
    
    // Increment the DescriptorNum 
    DescriptorNum += 2;
}

void Wad::createFile(const string &path){
}

int Wad::writeToFile(const string &path, const char *buffer, int length, int offset){
    return -1;
}





