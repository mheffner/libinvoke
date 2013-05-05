OBJ=hash_function.o hash_table.o invoke.o
LIB=libinvoke.so

$(LIB): $(OBJ)
	gcc -o $(LIB) -shared $(OBJ)
	ar cr libinvoke.a $(OBJ)

clean:
	rm -f *.o
