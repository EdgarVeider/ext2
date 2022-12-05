#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "header.h"

void ETX_read_superblock();
void EXT_read_groupBlockDescriptor();
void EXT_read_rootInode();