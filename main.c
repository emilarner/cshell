#include <stdio.h>
#include <sys/signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "config.h"
#include "cshell.h"
#include "text.h"

void help_menu()
{
    char *menu =
    "cshell %s - a simple Linux shell written in C as a proof of concept.\n"
    "USAGE:\n"
    "[no args] - run the shell normally, where you will be prompted for your commands.\n"
    "-h/--help - display this menu.\n"
    "[filename1] [filename2] ... - by providing a list of paths, interpret them sequentially.\n"
    "-e/--execute [cshell code] - execute code given afterwards.\n";
    "\nThe configuration file may be found/modified at ~/.cshell.cshl\n";

    printf(menu, CSHELL_VERSION);
}

int main(int argc, char **argv, char **envp)
{
    slist *args = slist_from_charpp(argv, argc);


    /* Prevent termination via stopping keys. */
    signal(SIGINT, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    struct interpreter *s = interpreter_init(envp);


    char config_file[256];
    snprintf(config_file, sizeof(config_file), "%s/" CSHELL_CONFIG_NAME, getenv("HOME"));

    /* Read configuration file. */
    FILE *fp = fopen(CSHELL_LOCAL_CONFIG ? "config.cshl" : config_file, "r");

    if (!fp)
        fprintf(stderr, "Cannot read your configuration file!: %s\n", strerror(errno));

    else
    {
        char config[256];
        while (fgets(config, sizeof(config), fp) != NULL)
            parse_line(s, config, false);
        
        fclose(fp);
    }

    /* Go through every argument passed to the program. */ 
    for (size_t i = 1; i < argc; i++)
    {
        /* Execute a line of code. */ 
        if (strequ(argv[i], "-e") || strequ(argv[i], "--execute"))
            parse_line(s, argv[i + 1], true);

        else if (strequ(argv[i], "--help") || strequ(argv[i], "-h"))
            help_menu();

        else
        {
            FILE *fp = fopen(argv[i], "r");

            if (!fp)
            {
                fprintf(stderr, "Error executing the file %s: %s\n", argv[i], strerror(errno));
                continue;
            }

            char line[256];
            memset(line, 0, sizeof(line));

            while ((fgets(line, sizeof(line), fp)) != NULL)
            {
                line[strlen(line) - 1] = '\0';
                parse_line(s, line, true);
            }

            free(fp);
        }
    }

    /* Endless loop of reading lines from the terminal */
    while (true)
    {
        char *prompt = g_hash_table_lookup(s->variables, "prompt");
        char formatted_prompt[512];
        if (prompt != NULL)
        {
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));

            snprintf(formatted_prompt, sizeof(formatted_prompt), prompt, 
                getenv("USER"),
                cwd
            );
        }

        /* With GNU readline, we can have history, completion, and movement of the arrow keys! */

        char *line;
        line = readline(prompt == NULL ? "cshell-noprompt>" : formatted_prompt);

        if (line == NULL)
            continue;

        parse_line(s, line, false);
        add_history(line);

        /* readline() returns a malloc'd string that need to be freed. */
        free(line);
    }

    interpreter_free(s);
    return 0;
}