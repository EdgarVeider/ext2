#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "header.h"

SuperBlock ETX_read_superblock();
GroupDesc EXT_read_groupBlockDescriptor();
void EXT_read_rootInode();
void EXT_contents_diretory();
static void read_dir(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group);