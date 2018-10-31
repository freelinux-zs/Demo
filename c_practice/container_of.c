#include <stdio.h>
#include <stdlib.h>
#define offsetof(TYPE,MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({	\
			const typeof( ((type *)0)->member) *__mptr = (ptr); \
			(type *) ((char *)__mptr - offsetof(type, member));})

#pragma  pack(4)

struct ptr
{
	char a;
	short b;
	int c;
	double d;
};

#pragma pack()


int main() {

	struct ptr Pt;
	struct ptr *pt;

	printf("ptr :%ld\n", sizeof(struct ptr));

	printf("&ptr :%p\n", &Pt);
	Pt.a = 'a';
	Pt.b = 2;
	Pt.c = 4;
	Pt.d = 12.04;

	pt = container_of(&Pt, struct ptr, a);

	printf("d:%d\n", pt->c);
	return 0;
}
