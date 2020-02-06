#include <stdarg.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <asm/byteorder.h>
#include <asm/page.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct FILE {
    int fd;
} FILE;

struct tm {
  int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
  int tm_min;                   /* Minutes.     [0-59] */
  int tm_hour;                  /* Hours.       [0-23] */
  int tm_mday;                  /* Day.         [1-31] */
  int tm_mon;                   /* Month.       [0-11] */
  int tm_year;                  /* Year - 1900.  */
  int tm_wday;                  /* Day of week. [0-6] */
  int tm_yday;                  /* Days in year.[0-365] */
  int tm_isdst;                 /* DST.         [-1/0/1]*/
}; 

void *sbrk(int increment);
void *malloc(size_t size);
void *memalign(size_t alignment, size_t n);
void free(void *ptr);
int printf(const char *fmt, ...);
int fprintf(FILE *f, const char *fmt, ...);
uint8_t *load_image(const char *filename);
void *realloc(void *ptr, size_t size);

int open(const char *filename, int access, ...);
int read(int fd, void *buf, size_t size);
int close(int fd);
long lseek(int fd, long offset, int whence);
int write(int fd, const void *buf, size_t size);

FILE *fopen(const char *path, const char *mode);
FILE *fdopen(int fildes, const char *mode);
int fclose(FILE *stream);
size_t fwrite(const  void  *ptr,  size_t  size,  size_t  nmemb,  FILE *stream);
int fputc(int c, FILE *stream);

long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
int atoi(const char *s);
float strtof(const char *nptr, char **endptr);
double strtod(const char *nptr, char **endptr);
long double strtold(const char *nptr, char **endptr);
double ldexp(double x, int exp);

int gettimeofday(struct timeval *tv, struct timezone *tz);
time_t time(time_t *t);
struct tm *localtime(const time_t *timep);

void exit(int val);
void getcwd(char *buf, size_t buf_size);

typedef int jmp_buf[6];

int setjmp(jmp_buf buf);
void longjmp(jmp_buf buf, int val);

int main(int argc, char **argv);
void fatal(const char *fmt, ...) __attribute__((noreturn)) ;
void romfs_init(void);
void show_filename(const char *filename);
void set_output_file(const char *filename,
                     uint8_t *base, size_t size);
long get_output_file_size(void);
void putstr(const char *s);
int do_gunzip(uint8_t *dest, const uint8_t *src, int src_len);

extern uint8_t *romfs_base;

extern int errno;
extern FILE *stderr;

