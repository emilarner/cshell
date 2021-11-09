
# cshell
A Linux shell interpreter written in C. Do not confuse with csh or any other programs of a similar name. 


cshell is a shell and/or scripting language written in C, using fork() and execvp() to create processes which execute non-builtin commands. cshell is similar--yet quite different--to Bash (and of the Slow Programming Language), providing many of the same functionalities. cshell is incomplete, so it only contains a minimal amount of abilities, like:

 - Quotation escaping 
 - If test expressions (based on the *test* command) like if-statements surrounded in hard brackets in Bash; additionally, else blocks.
 - Variables
 - Wildcards & Globbing 
 - Functions
 - Aliases
 - Tampering of environment variables
 - Output and input redirection of files
 - Pipes (coming soon)
 - Commenting (obviously), thereby shebangs (#!/usr/bin/cshell)
 - Command history (saving will come soon), tab completion, and arrow movement, thanks to GNU readline.
 - a lot of other things...

To quote cshell's help menu, which can be found by issuing -h or --help as a command-line parameter:

    cshell 1.1 - a simple Linux shell written in C as a proof of concept.
    USAGE:
    [no args] - run the shell normally, where you will be prompted for your commands.
    -h/--help - display this menu.
    [filename1] [filename2] ... - by providing a list of paths, interpret them sequentially.
    -e/--execute [cshell code] - execute code given afterwards.

As of right now, cshell does not come with a Makefile. If you want to compile cshell, please run the *make.sh* file provided in this repository. Additionally, if you wish to actually install this on your system for whatever reason, simply copy or symlink the binary to any of your paths. With the consideration that this is mostly a proof of concept and is relatively featureless, I wouldn't recommend it. 

In order to compile cshell, you need the following dependencies met:

 - glib, for hash tables (which is a quite bloated way of doing so)
 - readline, to provide a nice command-line interface like that of Bash and of many other shells.

cshell typically requires a configuration file at ~/.cshell.cshl--where you can see an example in the repository at *config.cshl*--but you may modify *config.h* to opt to have it be parsed locally, meaning the *config.cshl* file in the current directory for which cshell is running in. For production usage, this setting should be turned off. The configuration file is written in the scripting language itself, like Bash. 

Quotation escaping is how we can have spaces in our arguments, by using quotations to escape them. Any text that is enclosed in parentheses will be treated as one word, in spite of the fact that spaces may be within them. In cshell, quotation escaping is only possible with double quotes. It is best to always use quotation escaping whenever you are dealing with spaces, such as with *set*, *export*, and *alias*.

Commenting is quite trivial. You can either use the pound sign (#) or you can use two slashes (//) as you would in C++ or C--something that does not align with Bash or other scripting languages. Quite obviously, due to the fact that you can comment with pounds, you can use shebangs, in order to tell the kernel to execute a script file under a particular interpreter--shebangs are done at the kernel level! Due to the fact that any non-flag argument passed to cshell is merely executed as a script, as we will see more below, shebangs will naturally work. 

Variables are used by setting them with the *set* command and retrieving them by referencing them with a dollar sign, like Bash. Variables only act as a text-replacement tool, merely replacing themselves in the parser to be that of a certain text. In other words, they are not 'intelligent variables,' as you would see in other more sophisticated languages. Special variables which may be used to denote certain states will be prefixed with ~.

Aliases will replace the first words of a command with a predefined replacement--a common replacement is the replacement of `ls` to `ls --color=auto`. An alias definition may be made with the usage of the *alias* keyword, in a similar fashion to how variables are made. 

Environment variables may be set with the *export* command, which works like the *set* command. Environment variables set with *export* will be permanent throughout the duration of the program, naturally being passed to children of the shell. Temporary environment variable may be passed by providing a NAME=VALUE list before the name of an executable program, much like in Bash--these variables will only last for the duration of the command and all environment variables that were changed will be promptly resumed to their previous state.

Programs in path are run when the name of the command entered is not that of an alias, built-in, or functions. Internally, cshell will create a new child process with fork() and run an executable with execvp()--what all other shells do. The status or exit code afterwards will be stored in the special variable *$~code*. If one wishes to run subsequent programs, they may opt to use a semi-colon (not implemented right now). If one wishes to run subsequent programs but only if the previous one yielded a successful return code (0), then separate them with &. 


Statements and expressions may be run asynchronously--separate from the shell process, without blocking it--by the usage of the *async* keyword, where the statement or expression goes after it. 

Functions in cshell are declared with the *function* keyword and are in the style of Bash functions, where this line must be terminated with a () and without any defined positional parameters. All subsequent lines of code will be added to that function internally, until the *end* keyword is entered, where the function definition will stop. Parameters will be passed as if they were command-line arguments, being denoted by their number in the list like so: `$~0, $~1, $~2, ...`. If one wishes to obtain the entire line of parameters that was passed to the function, they may obtain it by accessing the special variable $~all. 

Input and output redirection to files is quite trivial in cshell and is similar to the procedure in Bash. In order to redirect the output of a command to a file, use >, like so: `neofetch>file.txt` (as of now, you cannot have a space). In order feed in the input of a file to the stdin of a command, use >, like so: `cat<file.txt` (again, no spaces). Specific file descriptors--such as stderr--will be supported, but currently are not. 

Piping is a way to streamline the output of one program into the input of another program down a chain. Of course, in cshell, we can accomplish this with the usage of the bar character (|); the method in which you pipe in cshell is exactly the same for Bash and even the Windows command-line. (Still, as of now, not actually implemented -- wait!)

If statements, for now, only support *test* commands. In other words, they execute the *test* command with the parameters you put into the if statement and check if the status code returns true. If statements that work off of the *test* command basis have their parameters enclosed in hard-brackets ([]) instead of parentheses, like Bash. If not being supplemented with an else-block, an if statement must be terminated with the *end* keyword--then that else-block will be terminated with it--similar to how blocks work in the Slow Programming Language. Example:

    if [-f /path/to/file]
	    echo "This file exists!"
	    neofetch
	else
		echo "Well, it doesn't exist..."
	end


Wildcards and globbing will work according to how glob works in general, where we are using the glob.h library to convert wildcards to a multitude of paths and files. This means that wildcarding and globbing will work in an identical fashion to Bash and many other shells. For instance, `echo *.c` in cshell will correctly echo all files with the extension--or rather, all files that includes .c in its name, to be more general--of .c, separated by spaces. The glob.h library is heavily indispensable here.
