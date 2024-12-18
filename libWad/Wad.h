#include <string>
#include <vector>
#include <unordered_map>
#include <fcntl.h>    // For open
#include <unistd.h>   // For read and close
#include <queue>
#include <sstream>
#include <functional>
using namespace std;
#define TreeName "WAD"
#include <iostream>

class TreeNode {
public:
    string Name;
    int content_offset;
    bool isDirectory;
    int Diroffset;
    int length;
    vector<TreeNode*> children; // Raw pointers to children
    bool isFirst = false;

    // Constructor for all other nodes
    TreeNode(const std::string& name, bool Directory, int length, int offset, int Doffset) {
        this->Name = name;
        this->isDirectory = Directory;
        this->length = length;
        this->content_offset = offset;
        this->Diroffset = Doffset;
    }

    // Constructor for the first node in the tree
    TreeNode(const std::string& name, bool Directory, bool first) {
        this->Name = name;
        this->isDirectory = Directory;
        this->isFirst = first;
    }

    // Destructor - Cleans up all children
    ~TreeNode() {
    // Recursively delete all children
    for (auto child : children) {
        delete child;
        child = nullptr;  // Avoid dangling pointers
    }
    children.clear();  // Clear the vector to free memory
    }

    // Add a child node to the current node
    void addChild(TreeNode* child) {
        children.push_back(child);
    }
};

class Tree {
private:
    TreeNode* root;  // Root node of the tree
    unordered_map<string, TreeNode*> NodeMap;  // Map of node names to nodes

public:
    // Constructor
    Tree(string name) {
        root = new TreeNode(name, true, true);  // Create a new root node
    }

    // Destructor
    ~Tree() {
        delete root;  // Delete root 
    }

    // Get root node
    TreeNode* getRoot() const {
        return root;
    }

    // Set root node
    void setRoot(TreeNode* node) {
        root = node;
    }

    // Add a child to the root
    void addChildToRoot(TreeNode* child) {
        root->addChild(child);
    }

    // Print the tree (simple DFS traversal)
    void printTree(TreeNode* node, int level = 0) const {
        std::cout << std::string(level, ' ') << node->Name << " " << node->Diroffset << std::endl;

        // Recurse through children
        for (const auto& child : node->children) {
            printTree(child, level + 2);  // Increase indentation for children
        }
    }

    // Print the entire tree starting from the root
    void print() const {
        printTree(root);
    }

    void BuildMap() {
    // Helper function for recursive traversal
    function<void(TreeNode*, string)> build = [&](TreeNode* node, string path) {
        NodeMap[path] = node; // Add the node to the map with the adjusted path
        for (TreeNode* child : node->children) {
            string childPath = path.empty() ? child->Name : path + "/" + child->Name;
            build(child, childPath);
        }
    };

    // Start building the map from root's children without including "root" in the path
    for (TreeNode* child : root->children) {
        build(child, child->Name);
    }
    }

    // Function that searches the map for the node
    TreeNode* SearchMap(string &path){
        // Setting the node to nothing
        TreeNode* FoundNode = nullptr;

        // Search the map 
        auto it = NodeMap.find(path);
        if(it != NodeMap.end()){
            FoundNode = it->second;
        }
        // return the found node
        return FoundNode;
    }

    // Function to insert into the Node map
    void InsertIntoMap(string path, TreeNode* node){
        NodeMap.insert({path, node});
    }

    // Function to count all directories in the tree starting from a given node, excluding the current node
    int countDirectories(TreeNode* node) {
        int count = 0;
        for (TreeNode* child : node->children) {
            if (child->isDirectory) {
                count++;
                count += countDirectories(child);
            }
        }
        return count;
    }

    // Function to count all content (non-directory nodes) in the tree starting from a given node, excluding the current node
    int countContent(TreeNode* node) {
        int count = 0;
        for (TreeNode* child : node->children) {
            if (!child->isDirectory) {
                count++;
            }
            else{
                count += countContent(child);
            }
            
        }
        return count;
    }

    int GetDiroffset(string path){
        return -1;
    }

    void PrintMap(){
        for(auto it = NodeMap.begin(); it != NodeMap.end(); it++){
            cout << it->first << " " << it->second->Name << endl;
        }
    }

};

class Wad {
    private:
        // Wad file data
        string magic;
        int DescriptorNum;
        int DescriptorOffset;

        // creating the tree
        Tree tree = Tree((string)TreeName); 
        

        // file path
        string filepath;       

        int fd; // File descriptor for the WAD file

    public:
        /**
         * Dynamically creates a Wad object and loads the WAD file data from path into memory.
         * Caller must deallocate the memory using the delete keyword.
         */
        static Wad* loadWad(const string &path);

        ~Wad() {
            // Clean up any dynamically allocated resources here
            close(fd);
        }

        /**
         * Returns the magic for this WAD data.
         * @return The magic string.
         */
        string getMagic();

        /**
         * Returns true if path represents content (data), and false otherwise.
         * @param path The path to check.
         * @return True if path represents content, false otherwise.
         */
        bool isContent(const string &path);

        /**
         * Returns true if path represents a directory, and false otherwise.
         * @param path The path to check.
         * @return True if path represents a directory, false otherwise.
         */
        bool isDirectory(const string &path);

        /**
         * If path represents content, returns the number of bytes in its data; otherwise, returns -1.
         * @param path The path to check.
         * @return The size of the content in bytes, or -1 if not content.
         */
        int getSize(const string &path);

        /**
         * If path represents content, copies as many bytes as are available, up to length, of content's data into the pre-existing buffer.
         * If offset is provided, data should be copied starting from that byte in the content.
         * Returns number of bytes copied into buffer, or -1 if path does not represent content (e.g., if it represents a directory).
         * @param path The path to the content.
         * @param buffer The buffer to copy data into.
         * @param length The maximum number of bytes to copy.
         * @param offset The offset to start copying from.
         * @return The number of bytes copied, or -1 if not content.
         */
        int getContents(const string &path, char *buffer, int length, int offset = 0);

        /**
         * If path represents a directory, places entries for immediately contained elements in directory.
         * The elements should be placed in the directory in the same order as they are found in the WAD file.
         * Returns the number of elements in the directory, or -1 if path does not represent a directory (e.g., if it represents content).
         * @param path The path to the directory.
         * @param directory The vector to place directory entries into.
         * @return The number of elements in the directory, or -1 if not a directory.
         */
        int getDirectory(const string &path, vector<string> *directory);

        /**
         * If given a valid path, creates a new directory using namespace markers at path.
         * The two new namespace markers will be added just before the “_END” marker of its parent directory.
         * New directories cannot be created inside map markers.
         * @param path The path to create the directory at.
         */
        void createDirectory(const string &path);

        /**
         * If given a valid path, creates an empty file at path, with an offset and length of 0.
         * The file will be added to the descriptor list just before the “_END” marker of its parent directory.
         * New files cannot be created inside map markers.
         * @param path The path to create the file at.
         */
        void createFile(const string &path);

        /**
         * If given a valid path to an empty file, augments file size and generates a lump offset, then writes length amount of bytes from the buffer into the file’s lump data.
         * If offset is provided, data should be written starting from that byte in the lump content.
         * Returns number of bytes copied from buffer, or -1 if path does not represent content (e.g., if it represents a directory).
         * @param path The path to the file.
         * @param buffer The buffer to write data from.
         * @param length The number of bytes to write.
         * @param offset The offset to start writing at.
         * @return The number of bytes written, or -1 if not content.
         */
        int writeToFile(const string &path, const char *buffer, int length, int offset = 0);

        void loadDiroffsets();
};

