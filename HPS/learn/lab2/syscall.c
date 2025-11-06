#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>

#undef errno
extern int errno;

// For heap management
extern char _end[]; // Defined by your linker script (linker.ld)
static char *heap_ptr = _end;

/* Adjusts the heap size (needed for malloc) */
caddr_t _sbrk(int incr) {
    char *prev_heap_ptr;
    prev_heap_ptr = heap_ptr;

    // Simple check: don't let the heap collide with the stack
    // In a full implementation, you'd check against memory boundaries.
    if (heap_ptr + incr > (char*)0x20000000) { // Example arbitrary boundary
        errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap_ptr += incr;
    return (caddr_t) prev_heap_ptr;
}

/* Writes data to file descriptor (needed for printf) */
int _write(int file, char *ptr, int len) {
    // Implement logic to send 'ptr' of 'len' bytes to your UART or debugger console
    // e.g., using a while loop and a function like hps_uart_send_byte(*ptr++)
    // For now, a non-functional stub:
    // This often needs to check if the file descriptor (file) is STDOUT (1) or STDERR (2).

    // Minimal non-functional stub:
    return len; // Pretend it worked
}

/* Reads from file descriptor (needed for scanf) */
int _read(int file, char *ptr, int len) {
    return 0; // Return 0 bytes read (non-functional)
}

/* Closes a file descriptor */
int _close(int file) {
    return -1; // Indicate error (non-functional)
}

/* Repositions read/write file offset */
off_t _lseek(int file, off_t ptr, int dir) {
    return (off_t)0; // Indicate no change (non-functional)
}

/* Gets file status information */
int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR; // Treat as a character device
    return 0;
}

/* Checks if file descriptor refers to a terminal */
int _isatty(int file) {
    // Return 1 for STDOUT/STDERR/STDIN, 0 otherwise.
    if (file == 1 || file == 2 || file == 0) return 1;
    return 0;
}
