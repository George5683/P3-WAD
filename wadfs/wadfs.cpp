#include "../libWad/Wad.h"
#include <string>
#include <fuse.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#define WADFS_ROOT_DIR "/"

#define WAD_FILE ((struct wad_s*) fuse_get_context()->private_data)->s

using namespace std;

struct wad_s {
    Wad* s;
};

// Get attributes for a file or directory
int wadfs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = time(NULL);
    stbuf->st_mtime = time(NULL);

    if (WAD_FILE->isDirectory(path)) {
        // Set permission to write and read
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (WAD_FILE->isContent(path)) {
        // Set permission to read and write
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = WAD_FILE->getSize(path);
    } else {
        return -ENOENT;
    }
    return 0;
}

// Create a directory
int wadfs_mkdir(const char *path, mode_t mode) {
    if (WAD_FILE->isContent(path)) {
        return -EEXIST;
    }

    WAD_FILE->createDirectory(path);
    return 0;
}

// Read from a file
int wadfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (!WAD_FILE->isContent(path)) {
        return -ENOENT;
    }

    int bytesRead = WAD_FILE->getContents(path, buf, size, offset);
    return (bytesRead >= 0) ? bytesRead : -EIO;
}

// Write to a file
int wadfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (!WAD_FILE->isContent(path)) {
        return -ENOENT;
    }

    int bytesWritten = WAD_FILE->writeToFile(path, buf, size, offset);
    return (bytesWritten >= 0) ? bytesWritten : -EIO;
}

// Release directory resources
int wadfs_releasedir(const char *path, struct fuse_file_info *fi) {
    if (WAD_FILE->isDirectory(path)) {
        fi->fh = 0;
        return 0;
    }
    return -ENOENT;
}

// Release file resources
int wadfs_release(const char *path, struct fuse_file_info *fi) {
    if (WAD_FILE->isContent(path)) {
        fi->fh = 0;
        return 0;
    }
    return -ENOENT;
}

// Read directory entries
int wadfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    vector<string> direc;

    struct stat s1;
    memset(&s1, 0, sizeof(struct stat));
    s1.st_uid = getuid();
    s1.st_gid = getgid();
    s1.st_atime = time(NULL);
    s1.st_mtime = time(NULL);
    s1.st_mode = S_IFDIR | 0555;
    s1.st_nlink = 2;

    if (WAD_FILE->isDirectory(path)) {
        filler(buf, ".", &s1, 0);
        filler(buf, "..", &s1, 0);
        WAD_FILE->getDirectory(path, &direc);

        for (const string &entry : direc) {
            filler(buf, entry.c_str(), NULL, 0);
        }
        return 0;
    }

    return -ENOENT;
}

// Create a new file
int wadfs_mknod(const char *path, mode_t mode, dev_t dev) {
    if (WAD_FILE->isContent(path)) {
        return -EEXIST;
    }

    WAD_FILE->createFile(path);
    return 0;
}

int wadfs_access(const char *path, int mask) {
    // Implement permission checking logic here. For example, return 0 if access is allowed.
    if (WAD_FILE->isContent(path)) {
        // Allow access based on your permission logic.
        return 0; // Access allowed
    } else if (WAD_FILE->isDirectory(path)) {
        // Allow access to directories.
        return 0; // Access allowed
    }

    return -EACCES; // Return permission denied if the path does not exist or access is not allowed.
}


static struct fuse_operations wadfs_operations = {
    .getattr = wadfs_getattr,   // The order should match the struct declaration in the FUSE library
    .mknod = wadfs_mknod,
    .mkdir = wadfs_mkdir,
    .read = wadfs_read,
    .write = wadfs_write,
    .release = wadfs_release,
    .readdir = wadfs_readdir,
    .releasedir = wadfs_releasedir,
    .access = wadfs_access
    
    // Add any additional FUSE operations as needed
};

int main(int argc, char *argv[]) {
    if(argc < 3){
        cout << "Not enough arguments" << endl;
        exit(EXIT_SUCCESS);
    }

    string wadPath = argv[argc - 2];

    //relative path
    if(wadPath.at(0) != '/'){
        wadPath = string(get_current_dir_name()) + "/" + wadPath;
    }

    cout << "Loading WAD file: " << wadPath << endl;

    struct wad_s wad;

    wad.s = Wad::loadWad(wadPath);

    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;


    // Run the FUSE main loop
    return fuse_main(argc, argv, &wadfs_operations, &wad);
}
