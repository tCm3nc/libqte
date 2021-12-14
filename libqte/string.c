/*******************************************************************************
Copyright (c) 2019-2020, Andrea Fioraldi


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <ctype.h>
#include "include/libqte.h"

void* __libqte_memcpy(void* dest, const void* src, size_t n) {
  unsigned char* d = dest;
  const unsigned char* s = src;

  if (!n)
    return dest;

  while (n--) {
    *d = *s;
    ++d;
    ++s;
  }

  return dest;
}

void* __libqte_memmove(void* dest, const void* src, size_t n) {
  unsigned char* d = dest;
  const unsigned char* s = src;

  if (!n)
    return dest;

  if (!((d + n) >= s && d <= (s + n)))  // do not overlap
    return __libqte_memcpy(dest, src, n);

  d = __libqte_malloc(n);
  __libqte_memcpy(d, src, n);
  __libqte_memcpy(dest, d, n);

  __libqte_free(d);

  return dest;
}

void* __libqte_memset(void* s, int c, size_t n) {
  unsigned char* b = s;
  while (n--)
    *(b++) = (unsigned char)c;
  return s;
}

void* __libqte_memchr(const void* s, int c, size_t n) {
  unsigned char* m = (unsigned char*)s;
  size_t i;
  for (i = 0; i < n; ++i)
    if (m[i] == (unsigned char)c)
      return &m[i];
  return NULL;
}

void* __libqte_memrchr(const void* s, int c, size_t n) {
  unsigned char* m = (unsigned char*)s;
  long i;
  for (i = n; i >= 0; --i)
    if (m[i] == (unsigned char)c)
      return &m[i];
  return NULL;
}

size_t __libqte_strlen(const char* s) {
  const char* i = s;
  while (*(i++))
    ;
  return i - s - 1;
}

size_t __libqte_strnlen(const char* s, size_t len) {
  size_t r = 0;
  while (len-- && *(s++))
    ++r;
  return r;
}

int __libqte_strcmp(const char* str1, const char* str2) {
  while (1) {
    const unsigned char c1 = *str1, c2 = *str2;

    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    str1++;
    str2++;
  }

  return 0;
}

int __libqte_strncmp(const char* str1, const char* str2, size_t len) {
  while (len--) {
    unsigned char c1 = *str1, c2 = *str2;

    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    str1++;
    str2++;
  }

  return 0;
}

int __libqte_strcasecmp(const char* str1, const char* str2) {
  while (1) {
    const unsigned char c1 = tolower(*str1), c2 = tolower(*str2);

    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    str1++;
    str2++;
  }

  return 0;
}

int __libqte_strncasecmp(const char* str1, const char* str2, size_t len) {
  while (len--) {
    const unsigned char c1 = tolower(*str1), c2 = tolower(*str2);

    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    str1++;
    str2++;
  }

  return 0;
}

int __libqte_memcmp(const void* mem1, const void* mem2, size_t len) {
  const char* strmem1 = (const char*)mem1;
  const char* strmem2 = (const char*)mem2;

  while (len--) {
    const unsigned char c1 = *strmem1, c2 = *strmem2;
    if (c1 != c2)
      return (c1 > c2) ? 1 : -1;
    strmem1++;
    strmem2++;
  }

  return 0;
}

int __libqte_bcmp(const void* mem1, const void* mem2, size_t len) {
  const char* strmem1 = (const char*)mem1;
  const char* strmem2 = (const char*)mem2;

  while (len--) {
    int diff = *strmem1 ^ *strmem2;
    if (diff != 0)
      return 1;
    strmem1++;
    strmem2++;
  }

  return 0;
}

char* __libqte_strstr(const char* haystack, const char* needle) {
  do {
    const char* n = needle;
    const char* h = haystack;

    while (*n && *h && *n == *h)
      n++, h++;

    if (!*n)
      return (char*)haystack;

  } while (*(haystack++));

  return 0;
}

char* __libqte_strcasestr(const char* haystack, const char* needle) {
  do {
    const char* n = needle;
    const char* h = haystack;

    while (*n && *h && tolower(*n) == tolower(*h))
      n++, h++;

    if (!*n)
      return (char*)haystack;

  } while (*(haystack++));

  return 0;
}

void* __libqte_memmem(const void* haystack,
                      size_t haystack_len,
                      const void* needle,
                      size_t needle_len) {
  const char* n = (const char*)needle;
  const char* h = (const char*)haystack;
  if (haystack_len < needle_len)
    return 0;
  if (needle_len == 0)
    return (void*)haystack;
  if (needle_len == 1)
    return memchr(haystack, *n, haystack_len);

  const char* end = h + (haystack_len - needle_len);

  do {
    if (*h == *n) {
      if (memcmp(h, n, needle_len) == 0)
        return (void*)h;
    }

  } while (h++ <= end);

  return 0;
}

char* __libqte_strchr(const char* s, int c) {
  while (*s != (char)c)
    if (!*s++)
      return 0;
  return (char*)s;
}

char* __libqte_strrchr(const char* s, int c) {
  char* r = NULL;
  do
    if (*s == (char)c)
      r = (char*)s;
  while (*s++);

  return r;
}

size_t __libqte_wcslen(const wchar_t* s) {
  size_t len = 0;

  while (s[len] != L'\0') {
    if (s[++len] == L'\0')
      return len;
    if (s[++len] == L'\0')
      return len;
    if (s[++len] == L'\0')
      return len;
    ++len;
  }

  return len;
}

wchar_t* __libqte_wcscpy(wchar_t* d, const wchar_t* s) {
  wchar_t* a = d;
  while ((*d++ = *s++))
    ;
  return a;
}

int __libqte_wcscmp(const wchar_t* s1, const wchar_t* s2) {
  wchar_t c1, c2;
  do {
    c1 = *s1++;
    c2 = *s2++;
    if (c2 == L'\0')
      return c1 - c2;

  } while (c1 == c2);

  return c1 < c2 ? -1 : 1;
}
