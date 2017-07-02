#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define CHECK_RETURN(s) { if(!s) { return; } }
#define CHECK_RETURN_VALUE(s, v) { if(!s) { return v; } }

#define ERROR_MESSAGE_LENGTH 4096
#define ERROR_FILE_NAME_LENGTH 128

typedef struct S_Error
{
    char file[ERROR_FILE_NAME_LENGTH];
    uint32 line;
    char message[ERROR_MESSAGE_LENGTH];
    char variables[ERROR_MESSAGE_LENGTH];
} S_Error;

S_Error last_error = { };
char *global_file;
uint32 global_line;

S_Error *GetLastError()
{
    return &last_error;
}

void SetLastError(char *file, uint32 line, char *message, char *variables, va_list vargs)
{
    CHECK_RETURN(file);
    CHECK_RETURN(message);
    CHECK_RETURN(variables);
    
    vsnprintf(last_error.variables, ERROR_MESSAGE_LENGTH, variables, vargs);

    strncpy(last_error.message, message, ERROR_MESSAGE_LENGTH);
    strncpy(last_error.file, file, ERROR_FILE_NAME_LENGTH);
    last_error.line = line;

}

void SetLastError(char *file, uint32 line, char *message, char *variables, ...)
{
    CHECK_RETURN(file);
    CHECK_RETURN(message);
    CHECK_RETURN(variables);
    
    va_list vargs;

    va_start(vargs, variables);

    vsnprintf(last_error.variables, ERROR_MESSAGE_LENGTH, variables, vargs);
    
    va_end(vargs);

    strncpy(last_error.message, message, ERROR_MESSAGE_LENGTH);
    strncpy(last_error.file, file, ERROR_FILE_NAME_LENGTH);
    last_error.line = line;

}

void SetLastError(char *message, char *variables, ...)
{
    CHECK_RETURN(message);
    CHECK_RETURN(variables);
    
    va_list vargs;

    va_start(vargs, variables);
    
    return SetLastError(global_file, global_line, message, variables, vargs);

    va_end(vargs);
}

#define SET_LAST_ERROR global_file = __FILE__; global_line = __LINE__; SetLastError

void PrintError(S_Error *err)
{
    CHECK_RETURN(err);
    
    printf("%s:%i | %s\n", err->file, err->line, err->message);
    printf("variables [ %s ]\n", err->variables);
}

void PrintLastError()
{
    PrintError(&last_error);
}

struct S_File
{
    FILE *f;
    bool open;
};

S_File *OpenFile(char *filename)
{
    CHECK_RETURN_VALUE(filename, 0);
    
    S_File *output = (S_File *)malloc(sizeof(S_File));

    if(output == 0)
    {
	SET_LAST_ERROR("Failed to allocate S_File", "size:%i", sizeof(S_File));
	return 0;
    }

    output->f = fopen(filename, "r");
    if(!output->f)
    {
	SET_LAST_ERROR("Failed to open file", "filename:\"%s\"", filename);
	free(output);
	return 0;
    }
	
    output->open = true;
    
    return output;
}

char *ReadFile(S_File *file)
{
    CHECK_RETURN_VALUE(file, 0);
    
    char buffer[4096];
    int s = fread(buffer, sizeof(char), 4096, file->f);
    if(s == 0)
    {
	SET_LAST_ERROR("Failed to read file", "buffer:\"%s\", file->f:%x", buffer, file->f);
	return 0;
    }
    
    char *output = (char *)malloc(sizeof(char) * s);
    if(!output)
    {
	SET_LAST_ERROR("Failed to allocate char *", "size:%i", sizeof(char) * s);
	return 0;
    }
    
    strcpy(output, buffer);

    return output;
}

void ReadAndPrintFile(char *filename)
{
    CHECK_RETURN(filename);

    S_File *f = OpenFile("../data/test.txt");
    if(!f)
    {
	PrintLastError();
	return;
    }

    char *message = ReadFile(f);
    if(!message)
    {
	PrintLastError();
	return;
    }
    
    printf("%s\n", message);
}

int main(int argc, char *argv)
{
    SET_LAST_ERROR("No Error", "", 0);
    ReadAndPrintFile("../data/test.txt");

    const int bad_error_input_size = 4096*50;
    char bad_error_input[bad_error_input_size];

    for(int i = 0; i < bad_error_input_size; i++)
    {
	bad_error_input[i] = 'a' + (i % ('z' - 'a'));
    }
    
    //SET_LAST_ERROR(bad_error_input, bad_error_input);
    //PrintLastError();
    
    getchar();
    
    return 0;
}
