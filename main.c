#include "./function_ext2.h"

#define FD_DEVICE "./myext2image.img" 

int main(){

    INODE inode;
    GroupDesc group_desc; 

    int fd = open(FD_DEVICE, O_RDONLY);

    read_inode(fd, 2, &group_desc, &inode);
    read_dir(fd, &inode, &group_desc);

    return 0;
}