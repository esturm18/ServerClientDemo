
#include<limits.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<time.h>
#include<ctype.h>
#include"myftp.h"
#include<sys/stat.h>
#include<sys/wait.h>

int main(int argc, char *argv[]){
        int numRead=0,errNum=0;
        char buf[BUF_SIZE]={0};
        struct maintenance storage;
        struct addrinfo information, *firstNode;
        memset(&information,0,sizeof(information));
        information.ai_family=AF_INET;
        information.ai_socktype=SOCK_STREAM;
        if(argc<3){
                printf("Usage: ./myftp [-d] <port> <hostname | IP address>\n");
                printf("Missing Argument\n");
                exit(0);
        }else if(argc==3){
                storage.debugFlag=0;
                storage.portNum=argv[1];
                storage.hostName=argv[2];
        }else if(argc==4){
                if(strcmp(argv[1],"-d")==0){
                        storage.debugFlag=1;
                        storage.portNum=argv[2];
                        storage.hostName=argv[3];
                }else{
                        printf("Usage: ./myftp [-d] <port> <hostname | IP address>\n");
                        printf("Incorrect command usage\n");
                        exit(1);
                }
        }else{
                printf("Usage: ./myftp [-d] <port> <hostname | IP address>\n");
                printf("Incorrect command usage\n");
                exit(1);
        }
        if(storage.debugFlag==1){
                printf("Debug output enabled\n");
        }
        errNum=getaddrinfo(storage.hostName,storage.portNum,&information,&firstNode);
        if(errNum!=0){
                fprintf(stdout,"Error: %s\n",gai_strerror(errNum));
                exit(1);
        }
        if(storage.debugFlag==1){
                printf("Hostname: %s and portNum: %s\n",storage.hostName,storage.portNum);
        }

        if((storage.sfd=socket(AF_INET,SOCK_STREAM,0))== -1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n", strerror(errNum));
                exit(1);
        }
        if(storage.debugFlag==1){
                printf("Created socket with descriptor %d\n", storage.sfd);
        }
        if((storage.cfd=connect(storage.sfd,firstNode->ai_addr,firstNode->ai_addrlen))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                exit(1);
        }
        if(storage.debugFlag==1){
                printf("Connected with server with descriptor %d\n", storage.cfd);
        }

        if(write(1,"Successfully connected to the server\n",37)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                exit(1);
        }

        struct sockaddr_in *addr_in=(struct sockaddr_in *)firstNode->ai_addr;
        inet_ntop(AF_INET,&addr_in->sin_addr,storage.IPaddress,sizeof(storage.IPaddress));

        if(storage.debugFlag==1){
                printf("Client is ready to receive commands\n");
        }

        while(1){
               commandLine(storage);
        }
        freeaddrinfo(firstNode);
        if(close(storage.sfd)==-1){
                errNum=errno;
                fprintf(stdout, "Error: %s\n", strerror(errNum));
                exit(1);
        }
        if(storage.debugFlag==1){
                printf("Client is exiting gracefully\n");
        }

        exit(0);
}

int commandLine(struct maintenance storage){
        char buf[MAXTERMINAL]={0};
        char *commands[MAXTERMINAL]={0};
        int i=0,j=0,errNum=0,numRead=0,emptyspace=0;
        if(write(1,"MYFTP> ",7)==-1){
                errNum=errno;
                fprintf(stdout, "%s\n", strerror(errNum));
                return 1;
        }
        while((numRead=read(0,&buf[i],1))>0 &&buf[i]!='\n'){
                i++;
        }
        if(numRead==-1){
                errNum=errno;
                fprintf(stdout, "%s\n", strerror(errNum));
                return 1;
        }
        buf[i]='\0';
        for(int i=0; i<strlen(buf);i++){
                if(isspace(buf[i])){
                        emptyspace++;
                }
        }
        if(emptyspace==strlen(buf)){
                return 1;
        }
        char *singleCommand=strtok(buf," ");
        while(singleCommand!=NULL){
                commands[j]=singleCommand;
                j++;
                singleCommand=strtok(NULL, " ");
        }
        commands[j+1]='\0';
        if(j>2){
                return 1;
        }
        for(int i=0; i<strlen(commands[0]);i++){
                 commands[0][i]=tolower(commands[0][i]);
        }
        if(strcmp(commands[0],"exit")==0){
                if(storage.debugFlag){
                        printf("Exit command encountered\n");
                }
                exitOperation(storage);
        }else if(strcmp(commands[0],"ls")==0){
                if(storage.debugFlag){
                        printf("Command string = %s\n", commands[0]);
                }
                lsOperation(storage);
        }else if(strcmp(commands[0],"rls")==0){
                if(storage.debugFlag){
                        printf("Command string = %s\n", commands[0]);
                        printf("Executing remote ls command\n");
                }
                rlsOperation(storage);
        }else if(strcmp(commands[0],"cd")==0){
                if(commands[1]==NULL){
                        write(1,"Error: Please enter a parameter\n",32);
                        return 1;
                }
                if(storage.debugFlag){
                        printf("Command string = '%s' with parameter = '%s'\n",commands[0],commands[1]);
                }
                cdOperation(commands[1],storage);
        }else if(strcmp(commands[0],"rcd")==0){
                if(commands[1]==NULL){
                        write(1,"Error: Please enter a parameter\n",32);
                        return 1;
                }
                if(storage.debugFlag){
                        printf("Command string = '%s' with parameter = '%s'\n",commands[0],commands[1]);
                }
                rcdOperation(commands[1],storage);
        }else if(strcmp(commands[0],"get")==0){
                if(commands[1]==NULL){
                        write(1,"Error: Please enter a parameter\n",32);
                        return 1;
                }
                if(storage.debugFlag){
                        printf("Command string 'get' with parameter ='%s'\n",commands[1]);
                }
                getOperation(commands[1],storage);
        }else if(strcmp(commands[0],"show")==0){
                if(commands[1]==NULL){
                        write(1,"Error: Please enter a parameter\n",32);
                        return 1;
                }
                if(storage.debugFlag){
                        printf("Command string = '%s' with parameter = '%s'\n",commands[0],commands[1]);
                }
                showOperation(commands[1],storage);
        }else if(strcmp(commands[0],"put")==0){
                putOperation(commands[1],storage);
        }else{
                printf("Invalid first command entered %s\n", commands[0]);
        }
        return 0;
}

int exitOperation(struct maintenance storage){
        char serverResponse[RESPONSE_SIZE]={0};
        int errNum=0;
        if(write(storage.sfd,"Q\n",2)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(storage.debugFlag){
                printf("Child is writing Q to the server\n");
        }
        if(read(storage.sfd,serverResponse,RESPONSE_SIZE)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(storage.debugFlag){
                printf("Awaiting server response\n");
                printf("Received server response: '%c'\n", serverResponse[0]);
        }
        exit(0);
}

int cdOperation(char *path, struct maintenance storage){
        int errNum=0;
        struct stat status;
        if(lstat(path,&status)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(!S_ISDIR(status.st_mode)){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(access(path,R_OK)!=0){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(chdir(path)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(storage.debugFlag){
                printf("Changed local directory to %s\n",path);
        }
        return 0;

}

int rcdOperation(char *path, struct maintenance storage){
        char buf[PATH_MAX+2]={0};
        char serverResponse[RESPONSE_SIZE]={0};
        int i=0,numRead=0,errNum=0;

        snprintf(buf,sizeof(buf),"C%s\n",path);

        if(write(storage.sfd,buf,strlen(buf))==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        while((numRead=read(storage.sfd,&serverResponse[i],1))>0&&serverResponse[i]!='\n'){
                i++;
        }
        if(serverResponse[0]=='E'){
                printf("%s",serverResponse+1);
                if(storage.debugFlag){
                        write(storage.cfd,serverResponse,strlen(serverResponse));
                        printf("Awaiting server response\n");
                        printf("Was not able to change remote directory in the server\n");
                }
                return 1;
        }
        if(numRead<0){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }

        if(serverResponse[0]=='A'){
                if(storage.debugFlag){
                        printf("Changed remote directory to %s\n",path);
                        printf("Awaiting server response\n");
                }
        }
        return 0;
}

int lsOperation(struct maintenance storage){
         int errNum=0;
         int fd[2];
         if(pipe(fd)==-1){
                 errNum=errno;
                 fprintf(stderr,"%s\n",strerror(errNum));
                 return 1;
         }
         int pid=fork();
         if(pid==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
         }
         if(pid==0){
                close(fd[rdr]);
                close(1);
                dup(fd[wtr]);
                close(fd[wtr]);
                if(storage.debugFlag){
                        printf("Client parent waiting on child process %d to run ls locally\n", getpid());
                        printf("Client child process %d executing local ls | more\n", getpid());
                        printf("Child process %d starting ls\n", getpid());
                }
                if(execlp("ls","ls","-la",NULL)==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
        }else{
                int pid2=fork();
                if(pid2==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
                if(pid2==0){
                        close(fd[wtr]);
                        close(0);
                        dup(fd[rdr]);
                        close(fd[rdr]);
                        if(storage.debugFlag){
                                printf("Child process %d starting more\n", getpid());
                        }
                        if(execlp("more","more","-d","-20",NULL)==-1){
                                errNum=errno;
                                fprintf(stderr,"%s\n",strerror(errNum));
                                return 1;
                        }
                }
                if(close(fd[wtr])==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
                if(close(fd[rdr])==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
                wait(NULL);
                wait(NULL);
        }
        if(storage.debugFlag){
                printf("Client parent continuing\n");
                printf("ls execution completed\n");
        }
        return 0;
}

int rlsOperation(struct maintenance storage){
        char serverResponse[RESPONSE_SIZE]={0};
        char aport[PORTNUM]={0};
        char *commandType="rls";
        struct sockaddr_in serverAddr;
        int i=0,ds=0,newAport=0,errNum=0,numRead=0;

        ds=getServerDataConnection(storage);

        if(storage.debugFlag){
                printf("Awaiting server response\n");
        }
        if(write(storage.sfd,"L\n",2)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(read(storage.sfd,serverResponse,RESPONSE_SIZE)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        if(storage.debugFlag){
                printf("Received server response: '%c'\n", serverResponse[0]);
                printf("Displaying response from server and forking to more\n");
        }
        if(serverResponse[0]=='E'){
                fprintf(stderr,"Error response from the server: %s", serverResponse+1);
                return 1;
        }
        int pid=fork();
        if(pid==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                close(ds);
                return 1;
        }
        if(pid==0){
                if(close(0)==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        close(ds);
                        return 1;
                }
                if(dup(ds)==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
                if(close(ds)==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
                if(storage.debugFlag){
                        printf("Waiting for child process %d to complete execution of more\n",getpid());
                }
                if(execlp("more","more","-d","-20",NULL)==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        return 1;
                }
        }else{
                wait(NULL);
        }
        if(storage.debugFlag){
                printf("Data display and more command completed\n");
        }
        return 0;
}

int getServerDataConnection(struct maintenance storage){
        int errNum=0,numRead=0,newAport=0,ds=0;
        struct sockaddr_in serverAddr;
        char aport[PORTNUM]={0};
        if(write(storage.sfd,"D\n",2)==0){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        numRead=read(storage.sfd,aport,sizeof(aport));
        if(numRead==-1) {
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        aport[numRead]='\0';
        newAport=atoi(aport+1);
        if(newAport==0) {
                fprintf(stderr, "Invalid port number in response: '%s'\n", aport);
                return 1;
        }
        if((ds=socket(AF_INET,SOCK_STREAM,0))==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        memset(&serverAddr,0,sizeof(serverAddr));
        serverAddr.sin_family=AF_INET;
        serverAddr.sin_port=htons(newAport);
        serverAddr.sin_addr.s_addr=inet_addr(storage.IPaddress);

        if(connect(ds,(struct sockaddr *)&serverAddr,sizeof(serverAddr))==-1) {
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Data connection to server established\n");
                printf("Awaiting server response\n");
        }
        return ds;
}

int getOperation(char *pathname,struct maintenance storage){
        char serverResponse[RESPONSE_SIZE]={0};
        char buf[DATANUM]={0};
        int errNum=0,numRead=0,newAport=0,ds=0,fd=0;
        ds=getServerDataConnection(storage);

        snprintf(buf,sizeof(buf),"G%s\n",pathname);
        if(write(storage.sfd,buf,strlen(buf))==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                close(ds);
                return 1;
        }

        if(read(storage.sfd,&serverResponse,RESPONSE_SIZE)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                close(ds);
                return 1;
        }
        int l=strlen(pathname);
        int count=0,i=l,flag=0,index=0;

        while(pathname[i]!='/'&&i>0){
                count++;
                i--;
        }
        if(i==0){
                index=0;
        }else{
                index=l-count;
                index+=1;
        }
        if(storage.debugFlag){
                printf("Getting %s from server and storing to %s\n",pathname+index,pathname+index);
        }
        int acc=access(pathname+index,F_OK);
        if(acc==0){
                printf("Open/create local file: file exists\n");
        }else if (serverResponse[0]!='E'){

                mode_t mode=O_CREAT|O_EXCL|O_WRONLY;
                mode_t mode2=S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

                fd=open(pathname+index,mode,mode2);
                if(fd==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        close(ds);
                        return 1;
                }
                while((numRead=read(ds,buf,DATANUM))>0){
                        write(fd,buf,numRead);
                }
                if(numRead==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errNum));
                        close(ds);
                        return 1;
                }
                close(fd);
        }else{
                fprintf(stderr,"Error response from server: %s",serverResponse+1);
                return 1;
        }
        if(close(ds)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errNum));
                return 1;
        }
        return 0;
}

int showOperation(char *pathname,struct maintenance storage){
        if(storage.debugFlag){
                printf("Showing file\n");
        }
        char aport[PORTNUM]={0};
        char buf[PATH_MAX+2]={0};
        char serverResponse[RESPONSE_SIZE]={0};
        struct sockaddr_in serverAddr;
        int numRead=0,newAport=0,ds=0,fd=0,errNum=0;
        ds=getServerDataConnection(storage);

        snprintf(buf,sizeof(buf),"G%s\n",pathname);
        if(write(storage.sfd,buf,strlen(buf))==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                return 1;
        }
        if(read(storage.sfd,&serverResponse,RESPONSE_SIZE)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                return 1;
        }
        if(storage.debugFlag){
                printf("Received server response '%c'\n", serverResponse[0]);
                printf("Displaying data from server and forking to more\n");
        }
        int l=strlen(pathname);
        int count=0,i=l,flag=0,index=0;
        while(pathname[i]!='/'&&i>0){
                count++;
                i--;
        }
        if(i==0){
                index=0;
        }else{
                index=l-count;
                index+=1;
        }
        if(!(access(pathname,F_OK)!=0 && serverResponse[0]=='E')){
                int pid=fork();
                if(pid==-1){
                        errNum=errno;
                        fprintf(stderr,"%s\n",strerror(errno));
                        return 1;
                }
                if(pid==0){
                        if(storage.debugFlag){
                                printf("Waiting for child process %d to complete execution of more\n", getpid());
                        }
                        if(close(0)==-1){
                                errNum=errno;
                                fprintf(stderr,"%s\n",strerror(errno));
                                return 1;
                        }
                        if(dup(ds)==-1){
                                errNum=errno;
                                fprintf(stderr,"%s\n",strerror(errno));
                                return 1;
                        }
                        if(close(ds)==-1){
                                errNum=errno;
                                fprintf(stderr,"%s\n",strerror(errno));
                                return 1;
                        }
                        if(execlp("more","more","-d","-20",NULL)==-1){
                                errNum=errno;
                                fprintf(stderr, "Error with executing more: %s\n", strerror(errNum));
                                return 1;
                        }
                }else{
                        wait(NULL);
                }
        }else{
                fprintf(stderr, "Error response from server: %s", serverResponse+1);
                return 1;
        }
        if(storage.debugFlag){
                printf("Data display and more command completed\n");
        }
        return 0;
}

int putOperation(char *pathname,struct maintenance storage){
        char aport[PORTNUM]={0};
        char buf[DATANUM]={0};
        char serverResponse[RESPONSE_SIZE]={0};
        struct sockaddr_in serverAddr;
        int numRead=0,newAport=0,fd=0,ds=0,errNum=0;

        if(storage.debugFlag){
                printf("Checking to make sure I have access to the file\n");
        }
        if(access(pathname,F_OK)!=0){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                return 1;
        }
        ds=getServerDataConnection(storage);

        if(storage.debugFlag){
                printf("Sending pathname to the server\n");
        }
        snprintf(buf,sizeof(buf),"P%s\n",pathname);

        if(storage.debugFlag){
                printf("Pathname is <%s>\n",buf);
        }
        int l=strlen(pathname);
        int count=0,i=l,flag=0,index=0,j=0;

        while(pathname[i]!='/' && i>0){
                count++;
                i--;
        }
        if(i==0){
                index=0;
        }else{
                index=l-count;
                index+=1;
        }
        if(storage.debugFlag){
                printf("Opening up the file for reading %s\n",pathname+index);
        }
        fd=open(pathname+index,O_RDONLY);
        if (fd==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                close(ds);
                return 1;
        }
        if(write(storage.sfd,buf,strlen(buf))==-1){
                errNum=errno;
                fprintf(stderr,"write%s\n",strerror(errno));
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Reading in server acknowledgement\n");
        }
        if(read(storage.sfd,&serverResponse,RESPONSE_SIZE)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Server acknowledgement is %s\n", serverResponse);
        }
        if(serverResponse[0]=='E'){
                printf("Server sent an error response, exiting put Operation\n");
                close(ds);
                return 1;
        }
        while((numRead=read(fd,buf,DATANUM))>0){
                write(ds,buf,numRead);
        }

        printf("Total number read was %d\n",numRead);

        if(numRead==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                close(ds);
                return 1;
        }
        close(fd);
        if(close(ds)==-1){
                errNum=errno;
                fprintf(stderr,"%s\n",strerror(errno));
                return 1;
        }
        return 0;
}
