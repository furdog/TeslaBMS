#ifndef ASYNC
typedef void * async;
#define ASYNC_CAT1(a, b) a##b
#define ASYNC_CAT(a, b) ASYNC_CAT1(a, b)
#define ASYNC_DISPATCH(state) void **_state = &state; \
			 if (*_state) { goto **_state; }
#define ASYNC_YIELD(act) do { *_state = &&ASYNC_CAT(_l, __LINE__); \
			      act; ASYNC_CAT(_l, __LINE__) :; } while (0)
#define ASYNC_AWAIT(cond, act) \
			 do { ASYNC_YIELD(); if (!(cond)) { act; } } while (0)
#define ASYNC_RESET(act) do { *_state = NULL; act; } while (0)
#define ASYNC
#endif
