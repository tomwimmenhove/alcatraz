/* 
 * This file is part of the alcatraz distribution (https://github.com/tomwimmenhove/alcatraz);
 * Copyright (c) 2021 Tom Wimmenhove.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* Taken from https://github.com/tomwimmenhove/toyos */

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
