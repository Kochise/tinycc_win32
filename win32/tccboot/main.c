#include "tccboot.h"
#include <linux/tty.h>
#include <asm/io.h>

#define TCCARGS_FILE "/boot/tccargs"
#define KERNEL_MAX_SIZE (8 * 1024 * 1024)
#define INITRD_MAX_SIZE (20 * 1024 * 1024)
#define INITRD_MIN_ADDR 0x800000
#define KERNEL_FILENAME "kernel"

//#define DEBUG_INITRD_ADDR

#define MAX_ARGS 1024

struct moveparams {
    uint8_t *low_buffer_start;  int lcount;
    uint8_t *high_buffer_start; int hcount;
};

static unsigned char *real_mode; /* Pointer to real-mode data */
#define EXT_MEM_K   (*(unsigned short *)(real_mode + 0x2))
#ifndef STANDARD_MEMORY_BIOS_CALL
#define ALT_MEM_K   (*(unsigned long *)(real_mode + 0x1e0))
#endif
#define SCREEN_INFO (*(struct screen_info *)(real_mode+0))
#define INITRD_START (*(unsigned long *) (real_mode+0x218))
#define INITRD_SIZE (*(unsigned long *) (real_mode+0x21c))

#define STACK_SIZE (256 * 1024)

long user_stack [STACK_SIZE];

struct {
    long * a;
    short b;
} stack_start = { & user_stack [STACK_SIZE] , __KERNEL_DS };


static char *vidmem = (char *)0xb8000;
static int vidport;
static int lines, cols;

void video_init(void)
{
	if (SCREEN_INFO.orig_video_mode == 7) {
		vidmem = (char *) 0xb0000;
		vidport = 0x3b4;
	} else {
		vidmem = (char *) 0xb8000;
		vidport = 0x3d4;
	}

	lines = SCREEN_INFO.orig_video_lines;
	cols = SCREEN_INFO.orig_video_cols;
}

static void scroll(void)
{
	int i;

	memcpy ( vidmem, vidmem + cols * 2, ( lines - 1 ) * cols * 2 );
	for ( i = ( lines - 1 ) * cols * 2; i < lines * cols * 2; i += 2 )
		vidmem[i] = ' ';
}

void putstr(const char *s)
{
	int x,y,pos;
	char c;

	x = SCREEN_INFO.orig_x;
	y = SCREEN_INFO.orig_y;

	while ( ( c = *s++ ) != '\0' ) {
		if ( c == '\n' ) {
			x = 0;
			if ( ++y >= lines ) {
				scroll();
				y--;
			}
                } else if (c == '\r') {
                    x = 0;
		} else {
			vidmem [ ( x + cols * y ) * 2 ] = c; 
			if ( ++x >= cols ) {
				x = 0;
				if ( ++y >= lines ) {
					scroll();
					y--;
				}
			}
		}
	}

	SCREEN_INFO.orig_x = x;
	SCREEN_INFO.orig_y = y;

	pos = (x + cols * y) * 2;	/* Update cursor position */
	outb_p(14, vidport);
	outb_p(0xff & (pos >> 9), vidport+1);
	outb_p(15, vidport);
	outb_p(0xff & (pos >> 1), vidport+1);
}

void exit(int val)
{
    printf("\n\n -- System halted");
    while (1);
}

char *tcc_args[MAX_ARGS];

static int expand_args(char ***pargv, const char *str)
{
    const char *s1;
    char **argv, *arg;
    int argc, len;

    argc = 0;
    argv = tcc_args;
    argv[argc++] = "tcc";
    for(;;) {
        while (isspace(*str))
            str++;
        if (*str == '\0')
            break;
        if (*str == '#') {
            while (*str != '\n')
                str++;
            continue;
        }
        s1 = str;
        while (*str != '\0' && !isspace(*str))
            str++;
        len = str - s1;
        arg = malloc(len + 1);
        memcpy(arg, s1, len);
        arg[len] = '\0';
        argv[argc++] = arg;
    }
    *pargv = argv;
    return argc;
}

void show_filename(const char *filename)
{
    int len;
    static int counter;
    char counter_ch[4] = "-\\|/";

    len = strlen(filename);
    if (len >= 2 && filename[len - 2] == '.' && filename[len - 1] == 'c') {
        printf("%c %-50s\r", counter_ch[counter], filename);
        counter = (counter + 1) & 3;
    }
}

int compile_kernel(struct moveparams *mv, void *rmode)
{
    int fd, len;
    char *args_buf;
    char **argv;
    int argc, ret, romfs_len;
    uint8_t *kernel_ptr, *initrd_ptr;
    unsigned long romfs_base1;
    
    real_mode = rmode;

    video_init();

    /* this is risky, but normally the initrd is not mapped inside the
       malloc structures. However, it can overlap with its new
       location */
    if (!INITRD_SIZE || !INITRD_START)
        fatal("the kernel source must be in a ROMFS filesystem stored as Initial Ram Disk (INITRD)");
    len = INITRD_SIZE;
    /* NOTE: it is very important to move initrd first to avoid
       destroying it later */
    initrd_ptr = memalign(16, len);
    memmove(initrd_ptr, (void *)INITRD_START, len);
    if (initrd_ptr[0] == 037 && ((initrd_ptr[1] == 0213) || 
                                 (initrd_ptr[1] == 0236))) {
        printf("Decompressing initrd...\n");
        romfs_base = memalign(16, INITRD_MAX_SIZE);
        romfs_len = do_gunzip(romfs_base, initrd_ptr, len);
        /* realloc it to minimize memory usage */
        romfs_base = realloc(romfs_base, romfs_len);
        free(initrd_ptr);
    } else {
        romfs_base = initrd_ptr;
        romfs_len = len;
    }

    kernel_ptr = malloc(KERNEL_MAX_SIZE);
    set_output_file("kernel", kernel_ptr, KERNEL_MAX_SIZE);

#ifdef DEBUG_INITRD_ADDR
    printf("romfs_base=%p romfs_len=%d kernel_ptr=%p\n", 
           romfs_base, romfs_len, kernel_ptr);
#endif
    printf("Compiling kernel...\n");

    fd = open("/boot/tccargs", O_RDONLY);
    if (fd < 0)
        fatal("Could not find '%s'", TCCARGS_FILE);
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    args_buf = malloc(len + 1);
    len = read(fd, args_buf, len);
    close(fd);

    args_buf[len] = '\0';
    argc = expand_args(&argv, args_buf);
    argv[argc] = NULL;
    free(args_buf);
#if 0
    {
        int i;
        for(i=0;i<argc;i++) {
            printf("%d: '%s'\n", i, argv[i]);
        }
    }
#endif
    ret = main(argc, argv);

    printf("%-50s\n", "");

    printf("Ok, booting the kernel.\n");
    
    mv->lcount = 0;
    mv->hcount = get_output_file_size();
    mv->high_buffer_start = kernel_ptr;

    /* relocate the uncompressed initrd so that the kernel cannot
       overwrite it */
    romfs_base1 = ((unsigned long)(mv->high_buffer_start) + 
                   mv->hcount + PAGE_SIZE - 1) & 
        ~(PAGE_SIZE - 1);
    if (romfs_base1 < INITRD_MIN_ADDR)
        romfs_base1 = INITRD_MIN_ADDR;
    if (!(kernel_ptr >= romfs_base + romfs_len &&
          (unsigned long)romfs_base >= INITRD_MIN_ADDR) &&
        (unsigned long)romfs_base < romfs_base1) {
        memmove((void *)romfs_base1, romfs_base, romfs_len);
        romfs_base = (void *)romfs_base1;
    }
#ifdef DEBUG_INITRD_ADDR
    printf("initrd_start=%p initrd_size=%d\n", romfs_base, romfs_len);
#endif
    INITRD_START = (unsigned long)romfs_base;
    INITRD_SIZE = romfs_len;
    return 1;
}
