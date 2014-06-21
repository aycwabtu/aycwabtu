#define STDC_HEADERS       1
#define HAVE_STDLIB_H      1
#define HAVE_STRING_H      1
#define HAVE_STRINGS_H     0
#define HAVE_INTTYPES_H    1
#define HAVE_STDINT_H      1

/* MSVC does not like      static inline void foo()
    ->    error C2054: expected '(' to follow 'inline'
    and accepts "_inline", but gcc does not like "_inline"...
    */
#if !defined(__GNUC__)
#define inline _inline
#endif
