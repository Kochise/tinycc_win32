#include "tccboot.h"

//#define ROMFS_DEBUG

char msg_buf[1024];

void puts(const char *s)
{
    putstr(s);
    putstr("\n");
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    va_end(ap);
    return 0;
}

FILE *stderr;

int fprintf(FILE *f, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    va_end(ap);
    return 0;
}

void getcwd(char *buf, size_t buf_size)
{
    strcpy(buf, "/");
}

void fatal(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    putstr("tccboot: panic: ");
    vsnprintf(msg_buf, sizeof(msg_buf), fmt, ap);
    putstr(msg_buf);
    putstr("\n");
    va_end(ap);
    exit(1);
}

/**********************************************************/

int errno;

long strtol(const char *nptr, char **endptr, int base)
{
    return simple_strtol(nptr, endptr, base);
}
 
long long strtoll(const char *nptr, char **endptr, int base)
{
    return simple_strtoll(nptr, endptr, base);
}

unsigned long strtoul(const char *nptr, char **endptr, int base)
{
    return simple_strtoul(nptr, endptr, base);
}
 
unsigned long long strtoull(const char *nptr, char **endptr, int base)
{
    return simple_strtoull(nptr, endptr, base);
}

int atoi(const char *s)
{
    return strtol(s, NULL, 10);
}

/**********************************************************/

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    tv->tv_sec = 0;
    tv->tv_usec = 0;
    return 0;
}

time_t time(time_t *t)
{
    if (t)
        *t = 0;
    return 0;
}

struct tm *localtime(const time_t *timep)
{
    static struct tm static_tm;
    return &static_tm;
}

int setjmp(jmp_buf buf)
{
    return 0;
}

void longjmp(jmp_buf buf, int val)
{
    exit(1);
}

/**********************************************************/
#define MALLOC_MAX_SIZE (128 * 1024 * 1024)
extern uint8_t _end;
uint8_t *malloc_ptr = &_end;

void *sbrk(int increment)
{
    uint8_t *ptr, *new_ptr;

    if (increment == 0)
        return malloc_ptr;
    ptr = malloc_ptr;
    new_ptr = malloc_ptr + increment;
    if (new_ptr > (&_end + MALLOC_MAX_SIZE)) {
        errno = ENOMEM;
        return (void *)-1;
    }
    malloc_ptr = new_ptr;
    return ptr;
}

#if 0
#define MALLOC_ALIGN 4096

void free(void *ptr)
{
}

void *realloc(void *oldptr, size_t size)
{
    void *ptr;
    if (size == 0) {
        free(oldptr);
        return NULL;
    } else {
        ptr = malloc(size);
        /* XXX: incorrect */
        if (oldptr) {
            memcpy(ptr, oldptr, size);
            free(oldptr);
        }
        return ptr;
    }
}
#endif
/**********************************************************/

uint8_t *romfs_base;

/* The basic structures of the romfs filesystem */

#define ROMBSIZE BLOCK_SIZE
#define ROMBSBITS BLOCK_SIZE_BITS
#define ROMBMASK (ROMBSIZE-1)
#define ROMFS_MAGIC 0x7275

#define ROMFS_MAXFN 128

#define __mkw(h,l) (((h)&0x00ff)<< 8|((l)&0x00ff))
#define __mkl(h,l) (((h)&0xffff)<<16|((l)&0xffff))
#define __mk4(a,b,c,d) htonl(__mkl(__mkw(a,b),__mkw(c,d)))
#define ROMSB_WORD0 __mk4('-','r','o','m')
#define ROMSB_WORD1 __mk4('1','f','s','-')

/* On-disk "super block" */

struct romfs_super_block {
	uint32_t word0;
	uint32_t word1;
	uint32_t size;
	uint32_t checksum;
	char name[0];		/* volume name */
};

/* On disk inode */

struct romfs_inode {
	uint32_t next;		/* low 4 bits see ROMFH_ */
	uint32_t spec;
	uint32_t size;
	uint32_t checksum;
	char name[0];
};

#define ROMFH_TYPE 7
#define ROMFH_HRD 0
#define ROMFH_DIR 1
#define ROMFH_REG 2
#define ROMFH_SYM 3
#define ROMFH_BLK 4
#define ROMFH_CHR 5
#define ROMFH_SCK 6
#define ROMFH_FIF 7
#define ROMFH_EXEC 8

/* Alignment */

#define ROMFH_ALIGN 16

#define MAX_FILE_HANDLES 256

typedef struct FileHandle {
    uint8_t *base;
    unsigned long size, max_size;
    unsigned long pos;
    int is_rw;
} FileHandle;

static FileHandle file_handles[MAX_FILE_HANDLES];

static uint8_t *output_base;
static size_t output_max_size, output_size;
static uint8_t output_filename[128];

void set_output_file(const char *filename,
                     uint8_t *base, size_t size)
{
    strcpy(output_filename, filename);
    output_base = base;
    output_max_size = size;
}

long get_output_file_size(void)
{
    return output_size;
}

static inline int get_file_handle(void)
{
    int i;

    for(i = 0; i < MAX_FILE_HANDLES; i++) {
        if (!file_handles[i].base)
            return i;
    }
    errno = ENOMEM;
    return -1;
}

int open(const char *filename, int access, ...)
{
    struct romfs_super_block *sb;
    unsigned long addr, next;
    struct romfs_inode *inode;
    int type, fd, len;
    char dir[1024];
    const char *p, *r;

    if (access & O_CREAT) {
        /* specific case for file creation */
        if (strcmp(filename, output_filename) != 0)
            return -EPERM;
        fd = get_file_handle();
        if (fd < 0)
            return fd;
        file_handles[fd].base = output_base;
        file_handles[fd].max_size = output_max_size;
        file_handles[fd].is_rw = 1;
        file_handles[fd].pos = 0;
        file_handles[fd].size = 0;
        return fd;
    }

    show_filename(filename);

    sb = (void *)romfs_base;
    if (sb->word0 != ROMSB_WORD0 ||
        sb->word1 != ROMSB_WORD1)
        goto fail;
    addr = ((unsigned long)sb->name + strlen(sb->name) + 1 + ROMFH_ALIGN - 1) &
        ~(ROMFH_ALIGN - 1);
    inode = (void *)addr;
    
    /* search the directory */
    p = filename;
    while (*p == '/')
        p++;
    for(;;) {
        r = strchr(p, '/');
        if (!r)
            break;
        len = r - p;
        if (len > sizeof(dir) - 1)
            goto fail;
        memcpy(dir, p, len);
        dir[len] = '\0';
        p = r + 1;
#ifdef ROMFS_DEBUG
        printf("dir=%s\n", dir);
#endif
        
        for(;;) {
            next = ntohl(inode->next);
            type = next & 0xf;
            next &= ~0xf;
            if (!strcmp(dir, inode->name)) {
#ifdef ROMFS_DEBUG
                printf("dirname=%s type=0x%x\n", inode->name, type);
#endif
                if ((type & ROMFH_TYPE) == ROMFH_DIR) {
                chdir:
                    addr = ((unsigned long)inode->name + strlen(inode->name) + 
                            1 + ROMFH_ALIGN - 1) &
                        ~(ROMFH_ALIGN - 1);
                    inode = (void *)addr;
                    break;
                } else if ((type & ROMFH_TYPE) == ROMFH_HRD) {
                    addr = ntohl(inode->spec);
                    inode = (void *)(romfs_base + addr);
                    next = ntohl(inode->next);
                    type = next & 0xf;
                    if ((type & ROMFH_TYPE) != ROMFH_DIR)
                        goto fail;
                    goto chdir;
                }
            }
            if (next == 0)
                goto fail;
            inode = (void *)(romfs_base + next);
        }
    }
    for(;;) {
        next = ntohl(inode->next);
        type = next & 0xf;
        next &= ~0xf;
#ifdef ROMFS_DEBUG
        printf("name=%s type=0x%x\n", inode->name, type);
#endif
        if ((type & ROMFH_TYPE) == ROMFH_REG) {
            if (!strcmp(p, inode->name)) {
                fd = get_file_handle();
                if (fd < 0)
                    return fd;
                addr = ((unsigned long)inode->name + strlen(inode->name) + 
                        1 + ROMFH_ALIGN - 1) &
                    ~(ROMFH_ALIGN - 1);
                file_handles[fd].base = (void *)addr;
                file_handles[fd].is_rw = 0;
                file_handles[fd].pos = 0;
                file_handles[fd].size = ntohl(inode->size);
                return fd;
            }
        }
        if (next == 0)
            break;
        inode = (void *)(romfs_base + next);
    }
 fail:
    errno = ENOENT;
    return -1;
}

int read(int fd, void *buf, size_t size)
{
    FileHandle *fh = &file_handles[fd];
    int len;

    len = fh->size - fh->pos;
    if (len > (int)size)
        len = size;
    memcpy(buf, fh->base + fh->pos, len);
    fh->pos += len;
    return len;
}

int write(int fd, const void *buf, size_t size)
{
    FileHandle *fh = &file_handles[fd];
    int len;
    
    if (!fh->is_rw)
        return -EIO;
    len = fh->max_size - fh->pos;
    if ((int)size > len) {
        errno = ENOSPC;
        return -1;
    }
    memcpy(fh->base + fh->pos, buf, size);
    fh->pos += size;
    if (fh->pos > fh->size) {
        fh->size = fh->pos;
        output_size = fh->pos;
    }
    return size;
}

long lseek(int fd, long offset, int whence)
{
    FileHandle *fh = &file_handles[fd];
    
    switch(whence) {
    case SEEK_SET:
        break;
    case SEEK_END:
        offset += fh->size;
        break;
    case SEEK_CUR:
        offset += fh->pos;
        break;
    default:
        errno = EINVAL;
        return -1;
    }
    if (offset < 0) {
        errno = EINVAL;
        return -1;
    }
    fh->pos = offset;
    return offset;
}

int close(int fd)
{
    FileHandle *fh = &file_handles[fd];
    fh->base = NULL;
    return 0;
}

/**********************************************************/

float strtof(const char *nptr, char **endptr)
{
    fatal("unimplemented: %s", __func__ );
}

long double strtold(const char *nptr, char **endptr)
{
    fatal("unimplemented: %s", __func__ );
}

double ldexp(double x, int exp)
{
    fatal("unimplemented: %s", __func__ );
}

FILE *fopen(const char *path, const char *mode)
{
    fatal("unimplemented: %s", __func__ );
}

FILE *fdopen(int fildes, const char *mode)
{
    FILE *f;
    f = malloc(sizeof(FILE));
    f->fd = fildes;
    return f;
}

int fclose(FILE *stream)
{
    close(stream->fd);
    free(stream);
}

size_t fwrite(const  void  *ptr,  size_t  size,  size_t  nmemb,  FILE *stream)
{
    int ret;
    if (nmemb == 1) {
        ret = write(stream->fd, ptr, size);
        if (ret < 0)
            return ret;
    } else {
        ret = write(stream->fd, ptr, size * nmemb);
        if (ret < 0)
            return ret;
        if (nmemb != 0)
            ret /= nmemb;
    }
    return ret;
}

int fputc(int c, FILE *stream)
{
    uint8_t ch = c;
    write(stream->fd, &ch, 1);
    return 0;
}
