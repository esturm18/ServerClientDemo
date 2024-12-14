# ServerClientDemo
Created by Emma Sturm 12/13/2024
Mini demo of a server and client model written in C
To compile the code, please move all three files into a directory on your computer terminal.
Then type in 'make' and it will create object files for both the client and server executable.

To run the server, you can type './myftpserve' onto the command line and it will automatically default to port 49999.
If you wish to use a different port, add a -p switch and then the port number, like this: './myftpserve -p 49998'.
To put the code in debug mode, which shows you the underlying code implementation, add a -d switch like this:
'./myftpserve -d' for port 49999 or './myftpserve -d -p 49998' for a different port. 

To run the client executable, type in './myftp portnumber hostname/IP address'. For example: './myftp 49999 localhost.'
The client executable also has a debug mode that can be utilized by entering in './myftp -d 49999 localhost'. 

Preferably, open the client and server on two seperate computers. It will also work on the same computer on two different terminals. The client has 7 different commands on which you can type after seeing the prompt "MYFTP> ". The seven commands are explained as follows:

1. "cd pathname" = Type in cd followed by a path you want to change your directory into. This will change the current directory of the client.
2. "ls" = To show the files listed in the directory of the client, enter in "ls"
3. "rcd pathname" = To change the directory of the server on the client side, type in rcd followed by the pathname.
4. "rls" = To showcase the files listed in the directory of the server, enter in "rls"
5. "get filename" = To get a file from the server side to the client side, type in "get" and then the filename you want that is on the server side.This will get the file from the server side and store in on the client's side. Type in "ls" to now see the added file.
6. "show filename" = To showcase the contents of a file on the server side, enter in this command. Hit the space bar to click forward 20 lines at a time, and q to quit.
7. "put filename" = To put a file that is on the client's side onto the server's, use this command. "Rls" to see the added file on the server side.

Thank you for viewing my mini client/server model. 
