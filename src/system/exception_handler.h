#ifndef __EXCEPTION_HANDLER_H_
#define __EXCEPTION_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OS_EXCEPTION_DSI                        2
#define OS_EXCEPTION_ISI                        3
#define OS_EXCEPTION_PROGRAM                    6

#define CPU_STACK_TRACE_DEPTH        10

#define __stringify_1(x)    #x
#define __stringify(x)        __stringify_1(x)

// http://elixir.free-electrons.com/linux/v2.6.24/source/include/asm-powerpc/reg.h#L713
#define mfspr(spr) \
({    register uint32_t _rval = 0; \
    asm volatile("mfspr %0," __stringify(spr) \
    : "=r" (_rval));\
    _rval; \
})

typedef struct _framerec {
	struct _framerec *up;
	void *lr;
} frame_rec, *frame_rec_t;

void setup_os_exceptions(void);

#ifdef __cplusplus
}
#endif

#endif
