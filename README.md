# Process-Finder
hw for UNIX programming
## Intro
- implement a simple process finder in C/C++ language. 
- enumerate all the processes in the system, just like the ps command. 
## Requirements
- list all the processes, and display the following information for each process: pid, uid, gid, ppid, pgid, sid, tty, status, and (image) command line.
- By default, listed only the processes belong to the current user and have an associated terminal.
- A -a option can be used to list processes from all the users.
- A -x option can be used to list processes without an associated terminal
- There are several sort options, -p, -q, -r, and -s, which sort the listed processes by pid (default), ppid, pgid, and sid, respectively.
