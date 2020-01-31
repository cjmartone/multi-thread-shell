# multi-thread-shell
This is a project to visualize threading by running a shell ontop of the linux shell. It can run commands in parallel either through the command line or when given a batch file.

## Using as a Prompt
To run the shell on its own, use the following commands.
```bash
gcc -o shell shell.c
./shell
```
From here, you can enter commands as you would in a normal terminal. To run them in parallel, put an & at the end of the command.
```bash
sleep 5&
```

## Using a script file
To feed the program a file containing scripts to run, compile and run as follows.
```bash
gcc -o shell shell.c
./shell [name of file]
```
Use a plain text file for this and the shell will begin executing the commands in the file.

## Using the example file
The example.c is used to show the efficiency of this multi thread shell. It populates a script file with commands to download every wikipedia page that has two letters in the name starting with AA and ending with ZZ. Then, example.c loads the script file with commands to parse each wikipedia page and count how many times every word is used across all the pages. At the end, it displays the results of this parse.

To run the example.c file, use the following commands.
```bash
gcc -o example example.c
./example.c [name of the script file]
```
The script file is ready to go. Refer to the 'Using a script file' section to run the shell with this file.
