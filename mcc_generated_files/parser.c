#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

ParsedData parseString(const char *input, char delimeter) {
    ParsedData result;
    result.count = 0;
    
    char *copy = strdup(input); // Make a copy of input because strtok modifies the string
    char *token = strtok(copy, delimeter);
    
    while (token != NULL && result.count < MAX_ARGS) {
        result.values[result.count] = strdup(token); // Store each token
        result.count++;
        token = strtok(NULL, delimeter);
    }
    
    free(copy); // Free the copied string
    return result;
}

void freeParsedData(ParsedData *data) {
    for (int i = 0; i < data->count; i++) {
        free(data->values[i]);
    }
}

//
//#include <stdio.h>
//#include "parser.h"
//
//int main() {
//    const char *testStrings[] = { "23,2F", "2300,AF00,03", "9600" };
//    int numTests = sizeof(testStrings) / sizeof(testStrings[0]);
//
//    for (int i = 0; i < numTests; i++) {
//        ParsedData result = parseString(testStrings[i]);
//        printf("Input: %s\nArguments Count: %d\n", testStrings[i], result.count);
//        
//        for (int j = 0; j < result.count; j++) {
//            printf("Argument[%d]: %s\n", j, result.values[j]);
//        }
//
//        freeParsedData(&result);
//        printf("\n");
//    }
//
//    return 0;
//}