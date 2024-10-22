How to compile the program:
To compile the program first clone the repo from github. Then you are going to cd into the repository and run the command “make”, then “./your inputs”. For example “./autograder (redirect, pipe, or exec) 5 1”.Breaking this down it is “./autograder <ipc_type> <p1> <p2> … <pn>”. For your ipc type, and p(n) are your parameters.

Any assumptions outside this document
Some assumptions outside of this document are that your computer can run C, you have a Linux system, and an understanding of cloning repositories/cd’ing into them.

Additionally to understand (since we are using pipes), knowledge in communicating information between the autograder and its children (executables). Also using I/O redirection (dup2) and random I/O (*seek) to detect infinite/blocked solutions with alarms.

Team id: P1 Group 49
Team member names: Cameron Nagle, Vivek Kethineni, Aaron Meyerhofer
X500’s: nagle118, kethi012, meye3630
