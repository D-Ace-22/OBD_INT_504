#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 10 // Maximum number of arguments that can be extracted

typedef struct {
    int count;                 // Number of arguments
    char *values[MAX_ARGS];    // Array of argument values
} ParsedData;

ParsedData parseString(const char *input, char delimeter);
void freeParsedData(ParsedData *data);

#endif