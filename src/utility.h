
#ifndef UTILITY_H_
#define UTILITY_H_


#define fail(text)										\
	do {												\
		fprintf(stderr, "Failure : %s\n", text);		\
		fprintf(stderr, "File : %s\n", __FILE__);		\
		fprintf(stderr, "Line : %d\n", __LINE__);		\
		fprintf(stderr, "Function : %s\n", __func__ );	\
		fflush(stderr);									\
		exit(EXIT_FAILURE);								\
	} while(0)


//#define PARSER_VERBOSE
#ifdef PARSER_VERBOSE
#define parser_verbose(printf_text, ...)				\
		fprintf(stdout, printf_text, __VA_ARGS__)
#else
#define parser_verbose(printf_text, ...)
#endif


//#define DELETE_VERBOSE
#ifdef DELETE_VERBOSE
#define delete_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define delete_verbose(...)
#endif



#endif /* UTILITY_H_ */
