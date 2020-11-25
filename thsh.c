#include "thsh.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char **argv, char **envp)
{
    bool finished = 0;   // flag that the program should end
    int input_fd = 0;    // default to stdin
    int ret = 0;         // return value
    bool debug_mode = 0; // debug flag

    // Add support for parsing the -d option from the command line
    // and handling the case where a script is passed as input to your shell

    if (argc > 1) // support for parsing the -d option. Debug flag set to 1
    {
        if (strcmp(argv[1], "-d") == 0)
            debug_mode = 1;

        else // support for the case where a script is passed as input to thsh
        {
            // Setting input descriptor to read from script
            char *script_name;
            script_name = strdup(argv[1]);

            input_fd = open(script_name, O_RDONLY);
            if (input_fd == -1)
            {
                printf("Error opening the file\n");
                return -errno;
            }
            if (argc > 2) // support for the case where a script is passed
            {             // as input to thsh and debug mode is on

                if (strcmp(argv[2], "-d") == 0)
                    debug_mode = 1;
            }
        }
    }

    // Initializong current directory
    ret = init_cwd();
    if (ret)
    {
        printf("Error initializing the current working directory: %d\n", ret);
        return ret;
    }

    // Initializing path table with path environment
    ret = init_path();
    if (ret)
    {
        printf("Error initializing the path table: %d\n", ret);
        return ret;
    }

    while (!finished)
    {
        int length;
        // Buffer to hold input
        char cmd[MAX_INPUT];
        // Get a pointer to cmd that type-checks with char *
        char *buf = &cmd[0];
        char *parsed_commands[MAX_PIPELINE][MAX_ARGS];
        char *infile = NULL;
        char *outfile = NULL;
        int pipeline_steps = 0;

        if (!input_fd)
        {
            ret = print_prompt();
            if (ret <= 0)
            {
                // If we printed 0 bytes, this call failed and the
                // program should end -- this will likely never occur
                finished = true;
                break;
            }
        }

        // Read a line of input
        length = read_one_line(input_fd, buf, MAX_INPUT);
        if (length <= 0)
        {
            ret = length;
            break;
        }

        // Pass it to the parser
        pipeline_steps = parse_line(buf, length, parsed_commands, &infile, &outfile);
        if (pipeline_steps <= 0)
        {
            printf("Parsing error. Cannot execute command. %d\n", -pipeline_steps);
            continue;
        }

        // Default settings
        int val = 0;       // return value from builtin command
        int rv = 0;        // return value when creating a pipe
        bool wait = false; // after fork, sets parent process to wait or not
        int std_in = 0;    // std_in is a file handle to be used as a param in run_command
        int std_out = 1;   // std_out is a file handle to be used as a param in run_command
        int in_file = 0;   // temp infile handle
        int out_file = 1;  // temp outfile handle

        // Redirection file
        if (infile)
        {
            // Read from file
            in_file = open(infile, O_RDONLY);
        }
        if (outfile)
        {
            // Write to file
            out_file = open(outfile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        }

        // Handle simple commands, redirection and piping
        int pipe_fd[2]; // array that holds pipe pointers

        for (int i = 0; i < pipeline_steps; i++) // going through each of the commands in parsed_commands
        {
            // Checking for debug flag
            if (debug_mode) fprintf(stderr, "RUNNING: [%s]\n", *parsed_commands[i]);

            if (i == 0) // first command in pipe, only reads if needed, writes
            {           // to new pipe if pipeline_steps > 1, no need to wait

                std_in = in_file; // read from file if needed

                if (pipeline_steps > 1)
                {
                    rv = pipe(pipe_fd);   // create new pipe
                    std_out = pipe_fd[1]; // write to pipe
                    wait = false;
                }
            }
            if (i == pipeline_steps - 1) // last command in pipe, read from old pipe if 
            {                            // pipeline_steps > 1, output to file if needed, needs to wait

                std_out = out_file; // write to file if needed
                wait = true;

                if (pipeline_steps > 1)
                {
                    std_in = pipe_fd[0]; // read from old pipe
                }
            }
            if ((i != 0) && (i != pipeline_steps - 1)) // command in the middle of pipe, reads from
            {                                          // old pipe, writes to new pipe, no need to wait

                std_in = pipe_fd[0];  // read from old pipe
                rv = pipe(pipe_fd);   // create new pipe
                std_out = pipe_fd[1]; // write to new pipe
            }

            // Run command if no errors opening files or pipes
            if ((rv == 0) && (std_in != -1) && (std_out != -1))
            {
                // Handling builtin commands
                ret = handle_builtin(*parsed_commands, input_fd, out_file, &val);

                if (ret == 1) // if it is a builtin command, then ret = val returned by builtin command
                {
                    ret = val;
                }
                else // not a builtin command
                {
                    ret = run_command(parsed_commands[i], std_in, std_out, wait);
                }

                // Closing pipes, if any
                if ((pipeline_steps > 1) && (i < pipeline_steps - 1))
                {
                    close(std_out);
                }
            }
            else // error opening files or pipes, set ret to -errno
            {
                ret = -errno;
                break;
            }

            // If error returned when running command, break loop
            if (ret) break;

            // Debug mode ending
            if (debug_mode) fprintf(stderr, "ENDED: [%s] (ret=%d)\n", *parsed_commands[i], ret);
        }

        // Close file handlers if exist
        if (infile) close(in_file);
        if (outfile) close(out_file);

        // Do not change this if/printf
        if (ret) printf("Failed to run command - error %d\n", ret);
    }
    return ret;
}
