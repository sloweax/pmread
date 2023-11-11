#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

long int eftell(FILE *f)
{
	long int s = ftell(f);
	if (s == -1)
		die("ftell:");
	return s;
}

void efseek(FILE *f, long off, int whence)
{
	if (fseek(f, off, whence) != 0)
		die("fseek:");
}

FILE *efopen(const char *path, const char *mode)
{
	FILE *f = fopen(path, mode);
	if (f == NULL)
		die("fopen:");
	return f;
}

void efwrite(const void *b, size_t s, size_t n, FILE *f)
{
	if (fwrite(b, s, n, f) != n)
		die("fwrite:");
}

void efread(void *b, size_t s, size_t n, FILE *f)
{
	if (fread(b, s, n, f) != n)
		die("fread:");
}

void *emalloc(size_t s)
{
	void *p = malloc(s);
	if (p == NULL)
		die("malloc:");
	return p;
}
