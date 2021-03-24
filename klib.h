#ifndef KLIB_H
#define KLIB_H

#include <stdint.h>
#include <stddef.h>

extern "C"
{
	void *memset(void *s, int c, size_t n);
	void *memcpy(void *dest, const void *src, size_t n);
	void *memmove(void *dest, const void *src, size_t n);
	int memcmp(const void *s1, const void *s2, size_t n);
	size_t strlen(const char *s);
	int strcmp(const char *s1, const char *s2);
	int strncmp(const char *s1, const char *s2, size_t n);
	char *strdup(const char *s);
	char *index(const char *s, int c);
};


#endif /* KLIB_H */
