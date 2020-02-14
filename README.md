# smash
How a command is processed

0. Program starts up in either stdin mode or file mode (main->mainloop)
1. mainloop takes in a line and seperates it out based on syntax/spaces
2. mainloop passes the entire line to parsecommand
3. parsecommand checks that each command in the line is valid
4. parsecommand forks and passes the arguments of the command to runcommand
    * or if built in parsecommand runs the helper function  
5. runcommand runs the given command with the arguments
6. parsecommand continues to run all of the commands in the line until there are no more. It then waits for all children processes to end and gives control back to mainloop
