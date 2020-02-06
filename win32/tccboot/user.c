#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include <linux/unistd.h>

#define __NR_linux_open __NR_open
#define __NR_linux_lseek __NR_lseek
#define __NR_linux_read __NR_read
#define __NR_linux_write __NR_write
#define __NR_linux_close __NR_close
#define __NR_linux_exit __NR_exit

_syscall3(int,linux_open,const char *,filename,int,access,int,mode)
_syscall3(int,linux_lseek,int,fd,int,offset,int,whence)
_syscall3(int,linux_read,int,fd,void *,buf,int,size)
_syscall3(int,linux_write,int,fd,const void *,buf,int,size)
_syscall1(int,linux_close,int,fd)
_syscall1(int,linux_exit,int,val)

void exit(int val)
{
    linux_exit(val);
}

void putstr(const char *s)
{
    linux_write(1, s, strlen(s));
}

uint8_t *load_image(const char *filename)
{
    int fd, size;
    uint8_t *p;

    fd = linux_open(filename, O_RDONLY, 0);
    if (fd < 0) {
        return NULL;
    }
    size = linux_lseek(fd, 0, SEEK_END);
    linux_lseek(fd, 0, SEEK_SET);
    p = malloc(size + 15);
    p = (void *)((unsigned long)p & ~15);
    linux_read(fd, p, size);
    linux_close(fd);
    return p;
}

