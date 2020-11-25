#include "thsh.h"
#include <stdlib.h>
#include <fcntl.h>

struct builtin
{
    const char *cmd;
    int (*func)(char *args[MAX_ARGS], int stdin, int stdout);
};

static char old_path[MAX_INPUT];
static char cur_path[MAX_INPUT];

/* 
 * This function needs to be called once at start-up to initialize
 * the current path. This should populate cur_path. Returns zero on
 * success, -errno on failure.
 */
int init_cwd(void)
{
    if (getcwd(cur_path, sizeof(cur_path)))
    {
        strcpy(old_path, cur_path);
        return 0;
    }
    return -errno;
}

// Handle a cd command.
int handle_cd(char *args[MAX_INPUT], int stdin, int stdout)
{
    // Note that you need to handle special arguments, including:
    // "-" switch to the last directory
    // "." switch to the current directory; this should change the
    //     behavior of a subsequent "cd -"
    // ".." go up one directory
    //
    // Hint: chdir can handle "." and "..", but saving
    //       these results may not be the desired outcome...
    //
    // XXX: be sure to write test cases for ., .., and weirdness
    // XXX: test for errors in the output if a cd fails

    int retval = 0;

    // Handling two many arguments
    if (args[2])
    {
        printf("Too many arguments\n");
        return retval;
    }

    // Handling cd (no arguments)
    if (!args[1])
    {
        strcpy(old_path, cur_path);
        strcpy(cur_path, getenv("HOME"));
        retval = chdir(cur_path);
        return retval;
    }

    // Handling cd with arg .
    if (strcmp(args[1], ".") == 0)
    {
        strcpy(old_path, cur_path);
        return retval;
    }

    // Handling cd with arg ..
    if (strcmp(args[1], "..") == 0)
    {
        strcpy(old_path, cur_path);
        retval = chdir("..");
        if (getcwd(cur_path, sizeof(cur_path))) return retval;
        return -errno;
    }

    // Handling cd with arg -
    if (strcmp(args[1], "-") == 0)
    {
        char temp[MAX_INPUT];
        strcpy(temp, cur_path);
        strcpy(cur_path, old_path);
        strcpy(old_path, temp);
        retval = chdir(cur_path);
        return retval;
    }

    // Handling regular case, if not valid path, return -errno
    retval = chdir(args[1]);
    if (retval == 0)
    {
        strcpy(old_path, cur_path);
        if (getcwd(cur_path, sizeof(cur_path)) == NULL) return -errno;
    }
    else return -errno;
    return retval;
}

// Handle an exit command
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout)
{
    exit(0);
    return 0; // does not actually return
}

// Handle goheels command
int handle_goheels(char *args[MAX_ARGS], int stdin, int stdout)
{
    char *ch = (char *)malloc(6000 * sizeof(char));
    ch = "\n\n                                      ;;                                           \n                                 #╣▓╝ ╔@@@@m╖  ````                                \n                           `    ╓╥╖╦@▓╢╢▓╩╜╙,                                      \n                       ,╓╥m²` ╓▓╢╢╢╢▓╜╙                                            \n                    ╓@▓▀╙╓mⁿ @╢▓╝╙└         ▄███r                                  \n                    ╙@ ╔▓    ,╓wr       ██µ▐██            ``         `             \n            ` ,φ▓▓▓▓▓▓ └╙╜╙╙└'  ,▄⌐▐██▄▄ ██µ██▄;▄█¿     ╓╥@▓▓▓▓▓╨╨╨Mπw;  `         \n               ▓▓╜. ╙▓╣▓ç    ╓▄µ ██⌐██▌▀████¿▀▀▀▀▀└,╥@▓▓╣╣▓▄ç└╙╣╣▓w ▓æ,'           \n               ▐╣ ╓ç  ╙╣╣▓    ██µ ██ ██▄ └▀▀▀    ⁿ▓╣╣▓@╖ç╙▓╣╣╣▓▓╣╣╣▓╣╣╣@▓╗         \n            ` ▐╣ ]╢╢▓Ç └▓▄▄   ██▄,██▌ ▀▀    ▄▄████▄ ╙▓╣╣╣╣▓╣╣╣╣╣▌╙╣╣▓╚╣╣╣╣▓@╖      \n               ╣∩╢╢╢╢╕  ███▄   ▀▀▀▀▀   ;▄▄█████▄▄▄j█▄ ╙▓╣╣╣▓╙▓╣╣Γ ╟╣▌ └╣▓╙▓╣╣m `   \n             ╙▓  └└'  ╙▀███▄     ▄▄▄██████▀▀▀▀▀▀█████µ ▓╣╣▓ j╣▓╥@▓╣▓@▄░  ]╣▀╣ '    \n               ╙▓ ╙╩╝ ▄▄¿ ██████████████▀▀ ,▄▄▄▄¿▐█████▄ ╚╣Wg▓▓▀╙└└,└╙╙▀▓▓╖  ╟~    \n          ╓@▓▓@ ╙▓   ,███¿ ███████████▀.,▄██████████████▄  └└       g▓▓@╗,╙▓@.     \n           ╓▓▓╙╓▓╣▓  ; ,█U╙▀█▄ ╙,█▀▐██▀▀ ▄█████▀▀▄███████████▄   ]@  ▓╢╢╢╢▓m ▓▓    \n         ~ ▓▓ ▓▀╙;▄███ █▌ █▄ ╙████▄ └ ,▄██▀▀└,;, █████▀█████████▄▄ ╙* ╙╩╩╩╜   ▓▓   \n           ╟╣▓▓w⌠▀▀██▀ █ ▐█▌   ███████▀▀ ▄▄▀▀▀▀▌ ██████ ▀████████████▄▄  ^#@@ç ▐╣  \n          ` ╙▓╣╣╣▓ⁿ╓@g⌐▐▄▐     ▐████▄¿ ▄█▀█▄    ,███████▄ ,▀▀███▀▀▀███████▄▄ ▓╣▐╣  \n               ╙▓╗ ╫╣▓▀,▄▄▄▄▄▄███████▌ ▀█      ╓███▀▀└ ,,,,       ,,;▄▄▄███▀ ╫▓ ▓▌ \n          ╓╥R▓ç ╙╨▓╙╓▄ ▀▀██▀▀▀▀▀███████▄¿'  ;▄███▀,æ▓▓▓░╙▀╙▀▀╨w   █████▌╙ #▓╝ ╫▓   \n       ╙╨▓▓▓Nm╨╜   ▄████▄▄▄▄▄▄▄▄████████████████ /▓╨╩╜,╓@Ñ╩▓▓@w,   └▀└,╓@▓@   ▓▓   \n                ╓▄▄▄▄▄▄▄;;└▀▀▀▀████████████████▌ ╣╣╣▓@▓╙     º▓╣▓W   ╫╢╢╢▓╜ ╓▓╜    \n              ` ▐████████████▄▄▄ └▀▀████████████ ╚▓╙▓╣▌ ╬ j@╗   ╓▓▓╗  ╫╜,g▓▀  '    \n               . :▐███▄▄└▀▀▀█████▄, ╙▀██████████▄└  ▓╣W  ╩╣╢╢m  ╙╩▓▓  ╥▓▓╜  `      \n                   ▀█████▄▄▄▄▄██████▄  ▀███▀██████▄▄ ╙▀▓▓@▄╓;,,╓ ╟▓╣L              \n                  `  ╙▀███████████████▄j███∩╙██████████▄▄▄└└└└└. ╓║▓H              \n                        └▀▀██████▌,▀███████;▄███████▄▀▀▀▀,       ▓╣▓               \n                          ╓╓;  ;└└  └▀██████▀└ ,,└└╘      . '  @▓▓'                \n                         . ╙╬W╬╢╢ ,╬▓C ,;, ╓φ@╝,     `       `  └  '               \n                              ╙╙╩╬▓▓╣@#▓╩╜╙└                                       \n                                   ...                                             \n	     ______                  __    __                   __          __         \n	    /      \\                /  |  /  |                 /  |        /  |        \n	   /$$$$$$  | ______        $$ |  $$ | ______   ______ $$ | _______$$ |        \n	   $$ | _$$/ /      \\       $$ |__$$ |/      \\ /      \\$$ |/       $$ |        \n	   $$ |/    /$$$$$$  |      $$    $$ /$$$$$$  /$$$$$$  $$ /$$$$$$$/$$ |        \n	   $$ |$$$$ $$ |  $$ |      $$$$$$$$ $$    $$ $$    $$ $$ $$      \\$$/         \n	   $$ \\__$$ $$ \\__$$ |      $$ |  $$ $$$$$$$$/$$$$$$$$/$$ |$$$$$$  |__         \n	   $$    $$/$$    $$/       $$ |  $$ $$       $$       $$ /     $$//  |        \n 	    $$$$$$/  $$$$$$/        $$/   $$/ $$$$$$$/ $$$$$$$/$$/$$$$$$$/ $$/         \n\n\n\0";

    write(stdout, ch, strlen(ch));
    return 0;
}

static struct builtin builtins[] = {{"cd", handle_cd},
                                    {"exit", handle_exit},
                                    {"goheels", handle_goheels},
                                    {'\0', NULL}};

/* 
 * This function checks if the command (args[0]) is a built-in. If so,
 * call the appropriate handler, and return 1. If not, return 0.
 *
 * stdin and stdout are the file handles for standard in and standard out,
 * respectively. These may or may not be used by individual builtin commands.
 *
 * Places the return value of the command in *retval.
 *
 * stdin and stdout should not be closed by this command.
 *
 * In the case of "exit", this function will not return.
 */
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval)
{
    int rv = 0;

    if (strcmp(args[0], builtins[0].cmd) == 0) // cmd == cd
    {
        *retval = handle_cd(&args[0], stdin, stdout);
        return 1;
    }
    else if (strcmp(args[0], builtins[1].cmd) == 0) // cmd == exit
    {
        *retval = handle_exit(&args[0], stdin, stdout);
    }
    else if (strcmp(args[0], builtins[2].cmd) == 0) // cmd == goheels
    {
        *retval = handle_goheels(&args[0], stdin, stdout);
        return 1;
    }
    return rv;
}

/* 
 * This function initially prints a default prompt of:
 * thsh>
 *
 * In Lab 1, Exercise 3, you will add the current working
 * directory to the prompt. As in, if you are in "/home/foo"
 * the prompt should be:
 * [/home/foo] thsh>
 *
 * Returns the number of bytes written.
 */
int print_prompt(void)
{
    int ret = 0;
    // Print the prompt
    // file descriptor 1 -> writing to stdout
    // print the whole prompt string (write number of
    // bytes/chars equal to the length of prompt)
    const char *prompt = "thsh> ";

    write(1, "[", strlen("["));
    write(1, cur_path, strlen(cur_path));
    write(1, "] ", strlen("] "));

    ret = write(1, prompt, strlen(prompt));
    return ret;
}
