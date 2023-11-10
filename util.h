#pragma once

#include <stdio.h>

void die(const char *fmt, ...);
long int eftell(FILE *f);
void efseek(FILE *f, long off, int whence);
FILE *efopen(const char *path, const char *mode);
void efwrite(const void *b, size_t s, size_t n, FILE *f);
void efread(void *b, size_t s, size_t n, FILE *f);
void *emalloc(size_t s);
