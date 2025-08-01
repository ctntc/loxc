#include "vm.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *path)
{
    FILE *file;
    fopen_s(&file, path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = (size_t)ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(file_size + 1);

    if (buffer == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\",\n", path);
        exit(74);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);

    if (bytes_read < file_size)
    {
        fprintf(stderr, "Could nto read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

static void repl()
{
    char line[1024];
    for (;;)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static void run_file(const char *path)
{
    char *source = read_file(path);
    interpret_result_t result = interpret(source);

    free(source);

    if (result == INTERPRET_COMPILE_ERROR)
        exit(65);
    if (result == INTERPRET_RUNTIME_ERROR)
        exit(70);
}

int main(const int argc, const char *argv[])
{
    init_vm();

    if (argc == 1)
    {
        repl();
    }
    else if (argc == 2)
    {
        run_file(argv[1]);
    }
    else
    {
        fprintf(stderr, "Usage: loxc [path]\n");
        exit(64);
    }

    free_vm();

    return EXIT_SUCCESS;
}
