Ok first during compilation we get a few warnings when "make" command is run. This is nothing to worry about. run the code anyways and it will work correctly. No additional files or directories are present in my submission and this is my readme. SUbmission folder only has necessary header files, .c files as well as header files. To test the user is encouraged to create their own files.

I have printed some extra information after some commands are run for convenience.for example when seek -e is called on some directory and it is the only one with that name, I printed directory changed to that directory.

For part 6 all commands are in main.c. 2nd part is also taken care of in main.c. GetDirectory.c is for 1st part.

ChangeDirectory.c is for 3rd part

Reveal.c is for 4th part. In case a command such as reveal -al is used where a and l are both there, i print all files hidden and non-hidden

log.c is for 5th part. All commands even ones that cause errors are stored.

Proclore.c is for 7th part covering proclore command. (Virtual Memory is measured in bytes). Cases where permission is not granted to see files a message confirming the same is printed. Note that most process ids wont work. You willl have to find processes running in your computer and check

Seek.c is for 8th part covering seek command

IN part 6 if a background process finishes, it prints during the time some other foregroun process(told to run by the user) is running. This means that if the process completes but the user is typing a command, the completeion message will come only after user presses enter.
