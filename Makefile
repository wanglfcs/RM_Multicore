APIARY=..
all:
	Bgcc1 -Wall -quiet -std=c99 -fno-builtin -O2 -I${APIARY}/include main.c
	Bgcc1 -Wall -quiet -std=c99 -fno-builtin -O2 -I${APIARY}/include my_math.c
	Bgcc1 -Wall -quiet -std=c99 -fno-builtin -O2 -I${APIARY}/include task_scheduler.c
	Bas -x -datarota=2 -lst main.lst main.s
	Bas -x -datarota=2 -lst my_math.lst my_math.s
	Bas -x -datarota=2 -lst task_scheduler.lst task_scheduler.s
	Bld -o aabb.out -codebase=1000 -datafloat ${APIARY}/lib/base.o my_math.o task_scheduler.o main.o -L${APIARY}/lib -lmc -lc -lgcc
	Bimg aabb.out
	cp aabb.img ~/share/aaab
clean:
	rm -rf *.s *.o *.img *.out *.lst
