#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int main(){
    int fileDescritor;
    fileDescritor = open("myext2image.img", O_RDONLY);

    printf("%d", fileDescritor);
}