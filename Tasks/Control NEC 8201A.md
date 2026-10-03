For the purpose of this task you are investigating how to reliably operate an NEC PC-8201A computer by interacting with the VirtualT emulator through the socket interface.

You can use and as necessary extend the vt_process and vt_socket libraries.  The goal is not to create new tests per se, but if you want to create a seperate regression test module to maintain confidence in your changes to libraries that would make sense. You can also create a vt_interact library on top of vt_socket and vt_process you need to organize any higher level control logic you create.

The NEC PC-8201A computer has a 40x8 LCD screen and a keyboard. Generally the computer is in one of two modes: at the main menu which is operated by arrow keys, or running a program. The main menu lists programs and files. Navigating to an entry and hitting ENTER will launch the entry. If it's a file, it should launch the program associated with the file with the file open. If it's a program, it will launch the program.

Virtual T's socket interface gives you access to both in various ways. Explore the socket interface through provided documentation and make a series of experiments culminating in launching the BASIC interpreter, typing in a 10-line program you author, observing and confirming the output, exiting back to the main menu.

In the process of this research, gradually document your discoveries and findings of interest in a new documentation file Findings.md

Document any higher level control operations you create in vt_interact in Interact.md .

