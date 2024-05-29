
#ifndef UTILITY_H_
#define UTILITY_H_



#define fail(text)										\
	do {												\
		fprintf(stdout, "Failure: %s\n", text);		\
		fprintf(stdout, "File: %s\n", __FILE__);		\
		fprintf(stdout, "Line: %d\n", __LINE__);		\
		fprintf(stdout, "Function: %s\n", __func__ );	\
		fflush(stdout);									\
		exit(EXIT_FAILURE);								\
	} while(0)


#define warning(text)									\
	do {												\
		fprintf(stdout, "Warning: %s\n", text);			\
		fprintf(stdout, "File: %s\n", __FILE__);		\
		fprintf(stdout, "Line: %d\n", __LINE__);		\
		fprintf(stdout, "Function: %s\n", __func__ );	\
		fflush(stdout);									\
	} while(0)


//#define PARSER_VERBOSE
#ifdef PARSER_VERBOSE
#define parser_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define parser_verbose(...)
#endif


//#define DELETE_VERBOSE
#ifdef DELETE_VERBOSE
#define delete_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define delete_verbose(...)
#endif

//#define TMP_VERBOSE
#ifdef TMP_VERBOSE
#define tmp_verbose(...)								\
		fprintf(stdout, __VA_ARGS__)
#else
#define tmp_verbose(...)
#endif




#endif /* UTILITY_H_ */
