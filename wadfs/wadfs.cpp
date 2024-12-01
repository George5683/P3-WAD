#include "../libWad/Wad.h"
#include <string>
#include <fuse.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <cstring>
#include <errno.h>
#include <stdio.h>

#define WADFS_ROOT_DIR "/"
#define FUSE_USE_VERSION 26

using namespace std;

Wad* wad; // Global pointer to Wad object

static int wadfs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    
    if (string(path) == WADFS_ROOT_DIR) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (wad->isDirectory(path)) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
    } else if (wad->isContent(path)) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = wad->getSize(path);
    } else {
        return -ENOENT;
    }

    return 0;
}

static int wadfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    if (!wad->isDirectory(path)) {
        return -ENOENT;
    }

    vector<string> entries;
    wad->getDirectory(path, &entries);

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    for (const auto &entry : entries) {
        filler(buf, entry.c_str(), NULL, 0);
    }

    return 0;
}

static int wadfs_mknod(const char *path, mode_t mode, dev_t dev) {
    if (wad->isDirectory(path)) {
        return -EEXIST;
    }

    wad->createFile(path);
    return 0;
}

static int wadfs_mkdir(const char *path, mode_t mode) {
    if (wad->isDirectory(path)) {
        return -EEXIST;
    }

    wad->createDirectory(path);
    return 0;
}

static int wadfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (!wad->isContent(path)) {
        return -ENOENT;
    }

    int bytesRead = wad->getContents(path, buf, size, offset);
    return (bytesRead >= 0) ? bytesRead : -EIO;
}

static int wadfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    if (!wad->isContent(path)) {
        return -ENOENT;
    }

    int bytesWritten = wad->writeToFile(path, buf, size, offset);
    return (bytesWritten >= 0) ? bytesWritten : -EIO;
}

static struct fuse_operations wadfs_operations = {
    .getattr = wadfs_getattr,
    .mknod = wadfs_mknod,
    .mkdir = wadfs_mkdir,
    .read = wadfs_read,
    .write = wadfs_write,
    .readdir = wadfs_readdir
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

    Wad* myWad = Wad::loadWad(wadPath);

    argv[argc - 2] = argv[argc - 1];
    argc--;


    // Run the FUSE main loop
    return fuse_main(argc, argv, &wadfs_operations, myWad);
}
