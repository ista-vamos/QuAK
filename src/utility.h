
#ifndef QUAK_UTILITY_H_
#define QUAK_UTILITY_H_

#include <cstdio>
#include <cstdlib>


namespace {

#ifdef __linux__
static inline void _fail(const char *file, int line, const char *func, const char* text) __attribute__((noreturn));
#else
__declspec(noreturn)
#endif
static inline void _fail(const char *file, int line, const char *func, const char* text) {
	fprintf(stderr, "Failure: %s\n", text);
	fprintf(stderr, "File: %s\n", file);
	fprintf(stderr, "Line: %d\n", line);
	fprintf(stderr, "Function: %s\n", func );
	fflush(stderr);
	exit(EXIT_FAILURE);
}

} // anon. namespace

#define QUAK_FAIL(text) _fail(__FILE__, __LINE__, __func__, (text));



//#define PARSER_VERBOSE
#ifdef PARSER_VERBOSE
#define parser_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define parser_verbose(...)
#endif


// #define DELETE_VERBOSE
#ifdef DELETE_VERBOSE
#define delete_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define delete_verbose(...)
#endif

//#define NIC_VERBOSE
#ifdef NIC_VERBOSE
#define NIC_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define NIC_verbose(...)
#endif




#endif /* QUAK_UTILITY_H_ */
