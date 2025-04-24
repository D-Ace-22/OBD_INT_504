#ifndef PARSER_H
#define PARSER_H
#include <xc.h>
#define MAX_ARGS 20 // Maximum number of arguments that can be extracted

typedef struct {
    int count;                 // Number of arguments
    char values[MAX_ARGS][20];    // Array of argument values
} ParsedData;

ParsedData parseString(const char *input, char delimeter);
void freeParsedData(ParsedData *data);
char* my_strdup(const char* src);
#endif