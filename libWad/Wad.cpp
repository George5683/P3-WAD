#include "Wad.h"
#include <fcntl.h>    // For open
#include <unistd.h>   // For read and close
#include <iostream>
#include <stack>
#include <sstream>
#include <queue>
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
    int TenCount = 0;
    // Read each Descriptor and add them to the tree
    for(int i = 0; i < wad->DescriptorNum; i++){

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

        // Creating a Stack
        stack<TreeNode*> Stack;

        // Checking if the file name ends with _START
        if(Element_Name.find("_START") != string::npos){
            // Remove the _START from the name
            Element_Name = Element_Name.substr(0, Element_Name.size() - 6);
            bool Directory = true;

            // Create a new TreeNode
            TreeNode* Node = new TreeNode(Element_Name, Directory, Element_Length);

            if(Stack.empty()){
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
            }
            else{
                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);
            }
    
            // Push the Node to the Stack
            Stack.push(Node);
        }
        // Checking if the file name starts with E#M#
        else if(Element_Name.size() >= 4 && Element_Name[0] == 'E' && 
         isdigit(Element_Name[1]) && Element_Name[2] == 'M' && 
         isdigit(Element_Name[3])){
            // Create a TreeNode
            TreeNode* Node = new TreeNode(Element_Name, true, Element_Length);
            
            if(Stack.empty()){
                // Add the child to the root
                wad->tree.addChildToRoot(Node);
            }
            else{
                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);
            }

            // Push the Node to the Stack
            Stack.push(Node);

            TenCount = 10;
        }
        // Checking if the name ends with _END
        else if(Element_Name.find("_END") != string::npos){
            // Pop the Node from the stack
            Stack.pop();

        }
        // If just a normal file
        else{
            // Check if the TenCount is set
            if(TenCount != 0){
                // Create a TreeNode
                TreeNode* Node = new TreeNode(Element_Name, false, Element_Length);

                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);

                // Decrease the TenCount
                TenCount--;

                if(TenCount == 0){
                    // Pop the Node from the stack
                    Stack.pop();
                }
            }

            // Create a TreeNode
            TreeNode* Node = new TreeNode(Element_Name, false, Element_Length);

            // Check if the stack is empty
            if(Stack.empty()){
                // add the node to the root
                wad->tree.addChildToRoot(Node);
            }
            else{
                // Add the Node to the Node at the top of the stack
                Stack.top()->addChild(Node);
            }
            
        }
    }

    // Close the file
    close(fd);
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
    // If the node is found, check if it is a directory
    else{
        if(parts.size() == 1){
            return temp->length;
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

int Wad::getContents(const string &path, char *buffer, int length, int offset){
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

void Wad::createDirectory(const string &path){
}

void createFile(const string &path, char *buffer, int length){
}

int writeToFile(const string &path, char *buffer, int length){
}
