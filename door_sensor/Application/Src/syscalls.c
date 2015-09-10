
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include "stm32f1xx_hal.h"

#undef errno
extern int errno;
extern int _end;

caddr_t _sbrk(int incr)
{

	   extern char _ebss;
	    static char *heap_end;
	    char *prev_heap_end;

	    if (heap_end == 0)
	    {
	        heap_end = &_ebss;
	    }
	    prev_heap_end = heap_end;

	    char * stack = (char*) __get_MSP();
	    if (heap_end + incr > stack)
	    {
	 //       _write(STDERR_FILENO, "Heap and stack collision\n", 25);
	 //       errno = ENOMEM;
	        return (caddr_t) -1;
	        //abort ();
	    }

	    heap_end += incr;
	    return (caddr_t) prev_heap_end;

//	static unsigned char *heap = NULL;
//	unsigned char *prev_heap;
//
//	if (heap == NULL)
//	{
//		heap = (unsigned char *) &_end;
//	}
//	prev_heap = heap;
//
//	heap += incr;
//
//	return (caddr_t) prev_heap;
}
