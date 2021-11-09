#ifndef CSHELL_C
#define CSHELL_C

#include "cshell.h"

/* Set a variable (redundant) */
void set_var(struct interpreter *i, char *key, char *value)
{
    g_hash_table_insert(i->variables, key, value);
}

/* Get a variable (redundant) */
char *get_var(struct interpreter *i, char *key)
{
    return g_hash_table_lookup(i->variables, key);
}

/* Parse text and replace the text with special information if necessary. */
slist *resolve_variables(struct interpreter *in, char *source)
{
    slist *s = split(source, " ", 0);
    slist *result = slist_init();

    for (size_t i = 0; i < s->length; i++)
    {
        /* If the first word... */
        if (i == 0)
        {

            /* If it is within the aliases, assign it! */ 
            if (g_hash_table_contains(in->aliases, s->data[i]))
            {
                slist_push(result, g_hash_table_lookup(in->aliases, s->data[i]));
                continue;
            }
        }

        if (strchr(s->data[i], '*') != NULL)
        {
            char *path = s->data[i];

            glob_t files;
            glob(path, GLOB_PERIOD | GLOB_TILDE | GLOB_NOCHECK, NULL, &files);

            for (size_t i = 0; i < files.gl_pathc; i++)
                slist_push(result, files.gl_pathv[i]);

            globfree(&files);
        }

        else if (memequ(s->data[i], "~/", 2))
        {
            char home_dir[strlen(s->data[i]) + strlen(getenv("HOME"))];
            snprintf(home_dir, sizeof(home_dir), "%s/%s", getenv("HOME"), s->data[i] + 2);
            slist_push(result, home_dir);
        }

        else if (s->data[i][0] == '$' && in->function == NULL)
        {   
            /* Evaluate command. */
            if (s->data[i][1] == '(' && s->data[i][strlen(s->data[i]) - 1] == ')')
            {
                /* Strip the dollar sign and enclosing parentheses. */
                char command[512];
                strncpy(command, s->data[i]+2, strlen(s->data[i]+2) - 1);

                char output[512];
                int stdout_fd = dup(fileno(stdout));


                /* Redirect stdout to a pipe, so that we can read it. */
                int pipefd[2];
                pipe2(pipefd, 0);
                dup2(pipefd[1], fileno(stdout));

                /* Capturing the result of this! */
                parse_line(in, command, true);
            
                fflush(stdout);
                close(pipefd[1]);

                /* Bring it back, read what was sent, and chop off the newline and/or other characters. */
                dup2(stdout_fd, fileno(stdout));
                read(pipefd[0], output, sizeof(output)); 
                output[strchr(output, '\n') - output] = '\0';

                /* Now, push it! */
                slist_push(result, output);
            }

            /* Evaluate variable */
            else 
            {
                /* Get the variable name--omitting the dollar sign. */
                char *resolution = get_var(in, s->data[i] + 1);

                /*if (resolution == NULL)
                    fprintf(stderr, "Warning: variable '%s' does not exist!\n", s->data[i] + 1);
                    */

                /* Of course, append the result, opting to say '(null)' if it didn't exist. */
                slist_push(result, resolution == NULL ? "(null)" : resolution);
            }
        }
        else
            /* Normal words. */
            slist_push(result, s->data[i]);
    }

    slist_free(s);
    
    return result;
}

/* Initialize the interpreter structure, allocating the many resources it comes with! */
struct interpreter *interpreter_init(char **envp)
{
    struct interpreter *i = (struct interpreter*) malloc(sizeof(struct interpreter));
    
    i->envp = envp;

    /* The environment variables are null-terminated, so count up until we reach NULL. */
    i->envp_len = 0;
    for (; envp[i->envp_len] != NULL; i->envp_len++);
    i->envp_len++;

    i->lineno = 0;
    i->async = false;

    i->if_on = false;
    i->else_on = false;
    i->if_condition = false;

    /* A lot of hash tables! */
    i->variables = g_hash_table_new(g_str_hash, g_str_equal);
    i->aliases = g_hash_table_new(g_str_hash, g_str_equal);
    i->functions = g_hash_table_new(g_str_hash, g_str_equal);

    i->function = NULL;
    i->status = 0;

    i->temp_envs = NULL;
    i->temp_envs_prev = NULL;
    
    return i;
}

/* Free the interpreter and all of the resources it has consumed. */
void interpreter_free(struct interpreter *i)
{
    g_hash_table_destroy(i->variables);
    g_hash_table_destroy(i->aliases);
    g_hash_table_destroy(i->functions);

    //slist_free(i->temp_envs);

    free(i);
}

/* Parse a line of code. */
void parse_line(struct interpreter *in, char *line, bool segment)
{
    if (!segment)
        in->lineno++;

    line = trimwhitespace(wstrip(line));

    if (line[0] == '\0')
        return;

    /* Commenting */
    if (line[0] == '#')
        return;

    /* c++ style */
    if (memequ(line, "//", 2))
        return;

    /* Text parsing. We still need to add stripping. */
    char keyword[64];

    slist *pwords = resolve_variables(in, line);
    string *joined = slist_join(pwords, " ");
    slist *words = split(joined->data, " ", '"');


    string_free(joined);
    slist_free(pwords);

    strncpy(keyword, slist_get(words, 0), sizeof(keyword));

    /* Detect if-statements. */
    if (in->if_on)
    {
        /* If the if-statement failed, don't run the code--unless we're ending the if-block or doing else. */
        if (!in->if_condition) 
        {
            if (!strequ(keyword, "end") && !strequ(keyword, "else"))
                return;

            if (strequ(keyword, "end"))
            {
                in->if_on = false;
                in->else_on = false;
            }
        }
            
    }


    if (in->else_on)
    {
        if (!in->else_on && !strequ(keyword, "end"))
            return;

        if (strequ(keyword, "end"))
        {
            in->if_on = false;
            in->else_on = false;
        }
    }


    if (in->function != NULL)
    {
        /* 'end' keyword, from another language I wrote! */
        if (strequ(keyword, "end"))
        {
            //free(in->function);
            in->function = NULL;
            return;
        }

        slist_push((slist*) g_hash_table_lookup(in->functions, in->function), line);
        return; 
    }

    /* exit - Exit with a return code. */
    if (strequ(keyword, "exit"))
    {
        exit((words->length > 1) ? atoi(slist_get(words, 1)) : 0);
    }

    /* set - Set a variable within the shell (not an environment one) */
    else if (strequ(keyword, "set"))
    {
        if (words->length < 3)
            die("Usage: set [varname] [value].\n");

        char *name = strdup(slist_get(words, 1));
        char *value = strdup(slist_get(words, 2)); 

        set_var(in, name, value);
    }

    /* export - Set an environment variable to this program, thereby all of its children afterwards. */
    else if (strequ(keyword, "export"))
    {
        if (words->length < 3)
            die("Usage: export [varname] [value].\n");

        char *name = slist_get(words, 1);
        char *value = slist_get(words, 2);

        setenv(name, value, true);
    }

    /* cd - Change directories. */
    else if (strequ(keyword, "cd"))
    {
        if (words->length < 2)
            die("Usage: cd [path]; see manual\n");

        char *path = strip(line, "cd ");
        chdir(path);

        if (!!errno)
            fprintf(stderr, "Error changing directories to '%s': %s\n", path, strerror(errno));
    }

    /* alias - Make an alias. */
    else if (strequ(keyword, "alias"))
    {
        if (words->length < 3)
            die("Usage: alias [alias] [value]\n");

        char *name = strdup(slist_get(words, 1));
        char *value = strdup(slist_get(words, 2));

        g_hash_table_insert(in->aliases, name, value);
    }

    /* async - Execute a program independently, without blocking this program. */
    else if (strequ(keyword, "async"))
    {
        if (words->length < 2)
            die("Usage: async [command]\n");

        in->async = true;
        parse_line(in, strip(line, "async "), true);

        return;
    }

    else if (strequ(keyword, "import"))
    {
        char *file = strip(line, "import ");
        file[strlen(file) - 1] = '\0';

        FILE *fp = fopen(file, "r");

        /* Error while opening/reading the file. */
        if (!fp)
            die(strerror(errno));

        char line[512];
        while (fgets(line, sizeof(line), fp) != NULL)
        {
            /* Rid ourselves of the terrible newline fgets() gives us. */
            line[strlen(line) - 1] = '\0';
            parse_line(in, line, false);
        }

        fclose(fp);
    }

    /* If something, do lines of code until fi or else. */
    else if (strequ(keyword, "if"))
    {
        char *condition = strip(line, "if ");

        /* test conditions */ 
        if (condition[0] == '[')
        {
            if (condition[strlen(condition) - 1] != ']')
            {
                fprintf(stderr, "if [...] must be enclosed within hard brackets... completely!");
                return;
            }

            condition++;
            condition[strlen(condition) - 1] = '\0';

            char test[6 + strlen(condition)];
            snprintf(test, sizeof(test), "test %s", condition);

            //printf("Test: %s\n", test);

            parse_line(in, test, true);

            in->if_on = true;
            in->if_condition = atoi(get_var(in, "~code")) ? false : true;
            /*                                           ^^^ it's actually reversed 0 = exists */ 
        }

    }

    else if (strequ(keyword, "else"))
    {
        if (!in->if_on)
            die("Cannot pursue an else-block if there was no initial if-block!");

        /* If the if-condition was false, else should be on! */
        in->else_on = !in->if_condition;
        in->if_on = false; 
    }

    /* and operator, on the basis of a zero-status code */
    else if (strchr(line, '&'))
    {
        char first[256];
        if (memcpy_s(first, line, strchr(line, '&') - line, sizeof(first)) == NULL)
            die("mem error");

        parse_line(in, first, true);

        /* well we didn't get a status code of true so do not execute the subsequent commands. */
        if (in->status != 0)
            peace();
        
        parse_line(in, strchr(line, '&') + 1, true);
    }

    /* Temporary environment variable parsing! */ 
    else if (strchr(keyword, '=') != NULL)
    {
        char *value = strchr(keyword, '=') + 1;
        char varname[256];
        if (memcpy_s(varname, keyword, strchr(keyword, '=') - keyword, sizeof(varname)) == NULL)
            die("mem error");

        if (in->temp_envs == NULL)
        {
            in->temp_envs = slist_init();
            in->temp_envs_prev = slist_init();
        }

        slist_push(in->temp_envs, varname);
        slist_push(in->temp_envs_prev, getenv(varname));
        setenv(varname, value, true);

        parse_line(in, strip(strip(line, keyword), " "), true);
        peace();
    }

    else if (strequ(keyword, "function"))
    {
        char *after = strip(line, "function ");
        size_t after_len = strlen(after);

        /* Declaration of function must end in parentheses */
        if (after[after_len - 2] != '(' || after[after_len - 1] != ')')
            die("Function declaration requires termination with ().\n");

        char *name = strdup(after);
        name[after_len - 2] = '\0';
        /*                  ^^^ get rid of () */


        if (g_hash_table_contains(in->functions, name))
        {
            fprintf(stderr, "Cannot define function '%s'! ", name);
            die("Cannot redefine function!\n");
        }

        in->function = name;
        /* Insert the function name and make it correspond to a list of code lines. */
        g_hash_table_insert(in->functions, in->function, slist_init());

    }

    /* Multiple statements on one line function/operator. */
    else if (strchr(line, ';'))
    {
        slist *statements = split(line, ";", '"');

        for (size_t i = 0; i < statements->length; i++)
            parse_line(in, statements->data[i], true);


        slist_free(statements);
        peace();
    }

    /* Piping. */
    else if (strchr(line, '|'))
    {
        char *after = strchr(line, '|') + 1; 
        char before[512];

        if (memcpy_s(before, line, strchr(line, '|') - line, sizeof(before)) == NULL)
            die("mem error");


    }

    /* We are calling a function. */
    else if (g_hash_table_contains(in->functions, keyword))
    {
        char varname[32];

        /* Set up arguments. */
        for (size_t i = 1; i < words->length; i++)
        {
            sprintf(varname, "~%lu", i - 1);
            set_var(in, strdup(varname), slist_get(words, i));
            /*         ^^^^ PROBABLY, i should make an abstraction for these heap allocated strings
             because they have been causing me problems when I forget to heap-allocate them for the 
             hash table */
        }

        /* And then a variable which represents everything... */
        set_var(in, "~all", strip(strip(line, keyword), " "));

        slist *code = g_hash_table_lookup(in->functions, keyword);

        /* Parse each line of code added to the function. */
        for (size_t i = 0; i < code->length; i++)
            parse_line(in, slist_get(code, i), true);

        
    }

    /* File redirection. */
    else if (strchr(line, '>') != NULL)
    {
        char before[256];
        if (memcpy_s(before, line, strchr(line, '>') - line, sizeof(before)) == NULL)
            die("mem error");

        char *filename = strchr(line, '>')+1;

        FILE *fp = fopen(filename, "wb");

        if (!fp)
        {
            fprintf(stderr, "Error opening '%s' for output redirection: %s;", filename, strerror(errno));
            die(" exiting...\n");
        }

        int stdoutfd = dup(fileno(stdout));
        dup2(fileno(fp), fileno(stdout));


        parse_line(in, before, true);

        fclose(fp);
        dup2(stdoutfd, fileno(stdout));
    }

    /* File stdin reading. */
    else if (strchr(line, '<') != NULL)
    {
        char before[1024];
        if (memcpy_s(before, line, strchr(line, '<') - line, sizeof(before)) == NULL)
            die("mem error");

        char *filename = strchr(line, '<') + 1;
        FILE *fp = fopen(filename, "rb");

        if (!fp)
        {
            fprintf(stderr, "Error opening file '%s' for input: %s", filename, strerror(errno));
            die("; exiting...\n");
        }

        int stdinfd = dup(fileno(stdin));
        dup2(fileno(fp), fileno(stdin));

        parse_line(in, before, true);

        fclose(fp);

        dup2(stdinfd, fileno(stdin));
    }

    /* What was entered was not a built-in command, so it must be a program we have to execute. */
    else
    {
        pid_t process = fork();

        /* Child process */
        if (!process)
        {
            /* execvp requires a NULL-terminated array. */
            /* slist does not mark length by a NULL-terminator, so we have to do this! */

            char *arguments[(words->length + 1) * sizeof(char*)];
            for (size_t i = 0; i < words->length; 
                arguments[i] = words->data[i], i++);

            arguments[words->length] = NULL; 

            /* Execute the program. */
            execvp(keyword, arguments);

            /* execvp never returns if success... but if it does, then error */
            perror(keyword);
        }


        /* If not 'async', do not wait for the process to finish--not storing its return code. */
        if (!in->async)
        {
            int status;
            waitpid(process, &status, 0);

            if (WIFEXITED(status)) 
            {
                char status_code[4];
                snprintf(status_code, sizeof(status_code), "%d", WEXITSTATUS(status));

                /* Save status code as variable ~code */
                set_var(in, "~code", strdup(status_code));
                in->status = WEXITSTATUS(status);
            }
        }


        /* Reset the environment variables back to normal. In general, I did this very lazily. */
        if (in->temp_envs != NULL)
        {
            /* Reset environment variables to previous values */
            for (size_t i = 0; i < in->temp_envs_prev->length; i++)
                setenv(in->temp_envs->data[i], 
                    in->temp_envs_prev->data[i] == NULL 
                    ? "(null)" : in->temp_envs_prev->data[i], true);

            slist_free(in->temp_envs);
            slist_free(in->temp_envs_prev);

            in->temp_envs = NULL;
            in->temp_envs_prev = NULL;
        }

        /* Reset the program back to the norm. */
        in->async = false; 
    }

    /* Clean up! */
    slist_free(words);

}

#endif