#include <stdio.h>
#include <string.h>

#include "invoke.h"

static struct symbol_entry *find_symbol_func(invoke_func_t *func);
static struct symbol_entry *find_symbol_name(const char *name);

void _func_intercept(invoke_func_t *func) __attribute((noinline));
static int find_level(void *func, void *pframe, void *cframe);

int add_handler(const char *funname, invoke_hndler_t *handler)
{
	struct symbol_entry *temp;

	temp = find_symbol_name(funname);
	if (temp == NULL) {
		printf("can't add handler for function: %s\n", funname);
		return -1;
	}

	temp->handler = handler;
	return 0;
}

static invoke_func_t *log(invoke_func_t *func, void *args, void *pframe,
    void *cframe)
{
	struct symbol_entry *temp;
	int level, i;

	level = find_level(func, pframe, cframe);
	temp = find_symbol_func(func);
#ifdef DEBUG
	if (temp != NULL)
		printf("(%d)%s\tpframe: %p\tcframe: %p\n", level, temp->name, pframe, cframe);
	else {
		printf("(%d)%p\tpframe: %p\tcframe: %p\n", level, func, pframe, cframe);
		return func;
	}
#else
	for (i = 0; i < level; i++)
		printf("    ");
	if (temp != NULL)
		printf("%s\n", temp->name);
	else
		printf("%p\n", func);
#endif

	if (temp->handler != NULL) {
		invoke_data_t idata;

		idata.fun = temp->func;
		idata.funname = temp->name;
		idata.argstart = args;
		idata.prevframe = pframe;

		temp->handler(temp, &idata);
	}
	return temp->remap != NULL ? temp->remap : func;
}

/* Intercept calls to user functions. Call the adaptor function which
 * allows the recommender to view and change the passed function
 * parameters before the intercepted function is called.
 */
void _func_intercept(invoke_func_t *func)
{

	/* call logger */
	asm("movl %esp,%eax");		/* store %esp... */
	asm("addl $4,%eax");		/* calculate where the current %ebp should be */
	asm("pushl %eax");		/* current stack frame pointer */
	asm("movl (%ebp),%eax");	/* previous stack frame pointer */
	asm("pushl %eax");
	asm("leal (%ebp),%eax");	/* find pointer to actual arguments, which */
	asm("addl $12,%eax");		/* is second pseudo arg. to this function */
	asm("pushl %eax");
	asm("movl 8(%ebp),%eax");	/* 'fun' argument */
	asm("pushl %eax");
	asm("call log");		/* call log function */
	asm("addl $16,%esp");		/* back %esp up */
	/* The return address from the call to log is used as the
	 * addres to jump to. Therefore, 'log' must return a valid
	 * function address, otherwise everything will get hosed.
	 */

	/* Make it appear like this function was never called. This requires
	 * moving the return address up to where the first argument of
	 * this function exists, restoring %ebp and finally moving up %esp
	 * to cover the gap.
	 */
	asm("movl 4(%ebp),%edx");	/* grab the return address... */
	asm("movl %edx,8(%ebp)");	/* ...and put it where the 'fun' pointer is */
	asm("movl (%ebp),%edx");	/* save the previous frame pointer... */
	asm("movl %edx,%ebp");		/* ...and restore it as the current one */
	asm("addl $8,%esp");		/* back up %esp */

	/* jump to the location saved earlier, this is the function the
	 * user was previously calling. This function does not continue
	 * after here, as the called function will return to the original
	 * caller.
	 */
	asm("jmp *%eax");
}

/* The symbol table is hashed on function pointer and function name */
static struct hash_table_type **symbol_table_func = NULL;
static struct hash_table_type **symbol_table_name = NULL;

void init_symbols(const char *filename)
{
	FILE *fp;
	char buf[512];
	invoke_func_t *func;
	char *funname, *cp;
	struct symbol_entry *temp;

	if (symbol_table_func != NULL)
		/* Don't run this again */
		return;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Can't open symbol table %s\n", filename);
		return;
	}

	symbol_table_func = (struct hash_table_type **)create_hash_table(MAX_HASHTABLE_SIZE);
	symbol_table_name = (struct hash_table_type **)create_hash_table(MAX_HASHTABLE_SIZE);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';

		if (strlen(buf) == 0)
			break;

		cp = strtok(buf, " ");
		if (cp == NULL || strlen(cp) == 0)
			break;
		func = (invoke_func_t *)strtol(cp, NULL, 16);

		cp = strtok(NULL, " ");
		if (cp == NULL || strlen(cp) == 0)
			break;
		funname = strdup(cp);
		if (funname == NULL)
			break;

		temp = (struct symbol_entry *)malloc(sizeof(struct symbol_entry));
		if (temp == NULL) {
			free(funname);
			break;
		}

		temp->func = func;
		temp->name = funname;
		temp->handler = NULL;
		temp->remap = NULL;
		if (hash_insert(symbol_table_func, MAX_HASHTABLE_SIZE,
			(unsigned char *)&func, sizeof(func), temp) == -1) {
			break;
		}
		if (hash_insert(symbol_table_name, MAX_HASHTABLE_SIZE,
			(unsigned char *)temp->name, strlen(temp->name),
			temp) == -1) {
			break;
		}
#ifdef DEBUG
		printf("adding symbol: %p/%s\n", temp->func, temp->name);
#endif
	}

	fclose(fp);
}

static int tryinit = 1;

static struct symbol_entry *find_symbol_func(invoke_func_t *func)
{
	struct symbol_entry *temp;

	if (symbol_table_func ==  NULL) {
		if (!tryinit)
			return NULL;
		init_symbols("symdump.txt");
		tryinit = 0;
		if (symbol_table_func == NULL)
			return NULL;
	}

	return (struct symbol_entry *)hash_find(symbol_table_func, MAX_HASHTABLE_SIZE,
	    &func, sizeof(func));
}

static struct symbol_entry *find_symbol_name(const char *name)
{
	struct symbol_entry *temp;

	if (symbol_table_name == NULL) {
		if (!tryinit)
			return NULL;
		init_symbols("symdump.txt");
		tryinit = 0;
		if (symbol_table_name == NULL)
			return NULL;
	}

	return (struct symbol_entry *)hash_find(symbol_table_name, MAX_HASHTABLE_SIZE,
	    name, strlen(name));
}

#define MAX_STACK_DEPTH 256

static int currlevel = -1;
struct call_stack_entry {
	void *func, *pframe, *cframe;
};

static struct call_stack_entry call_stack[MAX_STACK_DEPTH] = { NULL, NULL, NULL };

static int find_level(void *func, void *pframe, void *cframe)
{
	int i;

	for (i = currlevel; i >= 0; i--) {
		if (call_stack[i].cframe == pframe) {
			i++;
			break;
		}
	}

	if (i < 0)
		i = 0;

	call_stack[i].func = func;
	call_stack[i].pframe = pframe;
	call_stack[i].cframe = cframe;
	
	return currlevel = i;
}
