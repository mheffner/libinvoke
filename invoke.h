#ifndef __INVOKE_H__
#define __INVOKE_H__

typedef void (invoke_func_t)(void);

typedef struct {
	invoke_func_t *fun;
	char *funname;
	void *argstart;
	void *prevframe;
} invoke_data_t;

struct symbol_entry;

typedef void (invoke_hndler_t)(struct symbol_entry *, const invoke_data_t *);

struct symbol_entry {
	invoke_func_t *func;
	invoke_func_t *remap;
	char *name;
	invoke_hndler_t *handler;
};


void init_symbols(const char *filename);
int add_handler(const char *funname, invoke_hndler_t *handler);

#define MAX_HASHTABLE_SIZE 256


#endif /* __INVOKE_H__ */
