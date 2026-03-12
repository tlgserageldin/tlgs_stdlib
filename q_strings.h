#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define S(lit) ((s8){.data = (char *)lit, .len = (ptrdiff_t)sizeof(lit) - 1})


typedef struct {
	char *data;
	ptrdiff_t len;
} s8;

typedef struct {
	s8 head;
	s8 tail;
	bool ok;
} snip;

// Interop / safety utilities
bool s8_is_null(s8 s) {
	if (s.data == NULL) {
		return true;
	}
	return false;
}

bool s8_is_empty(s8 s) {
	if (s.len == 0) {
		return true;
	}
	return false;
}

// Output / formatting
// write exactly s.len bytes (no '\0' assumed)
int s8_fwrite(FILE *out, s8 s) {

	if (s8_is_empty(s) || s8_is_null(s)) {
		return -1;
	}
	if (out == NULL) {
		return -1;
	}
	return fwrite(s.data, sizeof(s.data[0]), (size_t)s.len, out);
}

int s8_fputs(FILE *out, const s8 s) {

	int res = EOF;
	if (s8_is_empty(s) || s8_is_null(s)) {
		return res;
	}
	if (out == NULL) {
		return res;
	}

	// allocate + 1 char and then copy over
	s8 n = {0};
	n.len = s.len + 1;
	n.data = (char *) calloc(n.len, sizeof(unsigned char));

	for (int i = 0; i < s.len; ++i) {
		n.data[i] = s.data[i];
	}

	res = fputs((char *)n.data, out);
	return res;
}

int s8_print(s8 s) {

	int res = -1;
	if (s8_is_empty(s) || s8_is_null(s)) {
		return res;
	}

	// allocate + 1 char and then copy over
	s8 n = {0};
	n.len = s.len + 1;
	n.data = calloc(n.len, sizeof(unsigned char));

	for (int i = 0; i < s.len; ++i) {
		n.data[i] = s.data[i];
	}

	res = fprintf(stdout, (char *)n.data);
	return res;

}

int s8_eprint(s8 s) {

	int res = -1;
	if (s8_is_empty(s) || s8_is_null(s)) {
		return res;
	}

	// allocate + 1 char and then copy over
	s8 n = {0};
	n.len = s.len + 1;
	n.data = (char *) calloc(n.len, sizeof(unsigned char));

	for (int i = 0; i < s.len; ++i) {
		n.data[i] = s.data[i];
	}

	res = fprintf(stdout, (char *)n.data);
	return res;

}

// Comparisons / predicates
bool s8_eq(s8 a, s8 b); // length + memcmp
int s8_cmp(s8 a, s8 b); // lexicographic compare (<0, 0, >0)


// Copy-out / cs8ing interop (only when needed)
// copy + NUL; false if too small
bool s8_to_cstr(s8 s, char *buf, size_t bufcap) {

    if (s8_is_valid(s)) {
        return false;
    }
    // +1 for terminating null
    if (s.len + 1 > bufcap || buf == NULL) {
        return false;
    }

    for (ptrdiff_t i = 0; i < s.len; ++i) {
        buf[i] = s.data[i];
    }
    buf[s.len] = '\0';

    return true;
}

bool s8_eq(s8 a, s8 b) {

    if (a.len != b.len) {
        return false;
    } else if (a.data == b.data) {
        return true;
    } else if (!a.data || !b.data) {
        return false;
    }
    return !memcmp(a.data, b.data, a.len);

}

s8 slice(char *start, char *end) {

    s8 s = {0};
	s.data = start;
	s.len = start ? end - start : 0;
	return s;

}

snip cut(s8 s, char c) {

    snip n = {0};
	if (!s.len) {
		return n;
	} // null pointer case

	char *beggining = s.data;
	char *end = s.data + s.len;
	char *cut = beggining;

	for (; cut < end && *cut != c; cut++)
		;

	n.ok = cut < end;
	n.head = slice(beggining, cut);
	// if ok then we want one past the delim,
	// otherwise we hit the end so just return a 0 len s8
	n.tail = slice(n.ok ? cut + 1 : cut, end);

    return n;

}

// s8 is valid if points to something and has > 0 length
inline static bool s8_is_valid(s8 a) {
    return !s8_is_null(a) || !s8_is_empty(a);
}
