/* simple Hello World program on the QEMU serial port */

void puts(const char *s);

void _start(void)
{
    puts("Hello World\n");
    while (1);
}

void outb(int port, int val)
{
    asm("outb %b1, %w0" : : "d" (port), "a" (val));
}

unsigned char inb(int port)
{
    int val;
    asm("inb %w1, %b0" : "=a"(val) : "d" (port));
    return val;
}

void puts(const char *s)
{
    while (*s) {
        outb(0x3f8, *s++);
        while ((inb(0x3f8 + 5) & 0x60) != 0x60);
    }
}



