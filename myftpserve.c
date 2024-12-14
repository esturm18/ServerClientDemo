#include<arpa/inet.h>
#include<ctype.h>
#include<stdio.h>
#include<sys/un.h>
#include<sys/wait.h>
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
#include<sys/stat.h>
#include<fcntl.h>
#include"myftp.h"

void main(int argc, char *argv[]){
        struct sockaddr_in addr, caddr;
        struct maintenance storage;
        int caddr_len=sizeof(caddr);
        int numRead=0,errNum=0,totalClients=0;
        char buf[BUF_SIZE]={0},host[NI_MAXHOST]={0};

        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family=AF_INET;

        if(argc==1){
                addr.sin_port=htons(atoi("49999"));
                storage.debugFlag=0;
        }else if(argc==2){
                addr.sin_port=htons(atoi("49999"));
                if(strcmp(argv[1],"-d")==0){
                        storage.debugFlag=1;
                }else{
                        printf("Usage: ./myftpserve <-d> -p some_port\n");
                        exit(1);
                }
        }else if(argc==3){
                storage.debugFlag=0;
                if(strcmp(argv[1],"-p")==0){
                        addr.sin_port=htons(atoi(argv[2]));
                }else{
                        printf("Usage: ./myftpserve <-d> -p some_port\n");
                        exit(1);
                }
        }else if(argc==4){
                if(strcmp(argv[1],"-d")==0){
                        storage.debugFlag=1;
                }else{
                        printf("Usage: ./myftpserve <-d> -p some_port\n");
                        exit(1);
                }
                if(strcmp(argv[2],"-p")==0){
                        addr.sin_port=htons(atoi(argv[3]));
                }else{
                        printf("Usage: ./myftpserve <-d> -p some_port\n");
                        exit(1);
                }
        }else{
                printf("Usage: ./myftpserve <-d> -p some_port\n");
                exit(1);
        }

        if((storage.sfd=socket(AF_INET,SOCK_STREAM,0))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                printf("Server fatal error, exiting\n");
                exit(1);
        }

        if(storage.debugFlag){
                printf("Parent: Debug output enabled\n");
                printf("Parent: socket created with descriptor %d\n",storage.sfd);
        }
        setsockopt(storage.sfd,SOL_SOCKET,SO_REUSEADDR,&(int){1}, sizeof(int));

        if(storage.debugFlag){
                printf("Binding to sfd\n");
        }
        if((bind(storage.sfd,(struct sockaddr *)&addr,sizeof(addr)))==-1){
                errNum=errno;
                fprintf(stdout, "Error: %s\n", strerror(errNum));
                printf("Server fatal error, exiting\n");
                exit(1);
        }
        if(storage.debugFlag){
                printf("Listening for a client\n");
        }
        if((listen(storage.sfd,BACKLOG))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n", strerror(errNum));
                printf("Server fatal error, exiting\n");
                exit(1);
        }
        for(;;){
                if((storage.cfd=accept(storage.sfd,(struct sockaddr *)&caddr,&caddr_len))==-1){
                        errNum=errno;
                        fprintf(stdout,"Error: %s\n",strerror(errNum));
                        printf("Server fatal error, exiting\n");
                        exit(1);
                }
                if(storage.debugFlag){
                        printf("Client accepted with descriptor %d\n",storage.cfd);
                }

                if((errNum=getnameinfo((struct sockaddr *)&caddr,sizeof(caddr),host,NI_MAXHOST,NULL,0,0))!=0){//Service set to null, don't need here
                        fprintf(stdout, "Error: %s\n", gai_strerror(errNum));
                        printf("Server fatal error, exiting\n");
                        exit(1);
                }
                if(storage.debugFlag){
                        printf("Client hostname is: %s\n",host);
                }

                ++totalClients;
                if(storage.debugFlag){
                        printf("Parent: Listening with connection queue of %d\n",storage.debugFlag);
                        printf("Total clients are %d\n", totalClients);
                }
                fflush(stdout);
                fflush(stderr);

                int pid=fork();
                if(pid==-1){
                        errNum=errno;
                        fprintf(stderr,"Error: %s\n",strerror(errNum));
                        printf("Server fatal error, exiting\n");
                        exit(1);
                }
                if(pid==0){
                        printf("Child %d started\n",getpid());
                        printf("Child %d connection accepted by host %s\n",getpid(), host);
                        close(storage.sfd);
                        if(serverCommands(storage)==-1){
                                exit(1);
                        }
                        close(storage.cfd);
                        printf("Child %d leaving the connection\n",getpid());
                        exit(0);
                }else{
                        close(storage.cfd);
                        while(waitpid(0,NULL,WNOHANG)>0){
                                continue;
                        }
                }
        }

        if(close(storage.sfd)==-1){
                errNum = errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                printf("Server fatal error, exiting\n");
                exit(1);
        }
        if(storage.debugFlag){
                printf("Server exiting gracefully\n");
        }
        exit(0);
}

int serverCommands(struct maintenance storage){
        int ds=0,i=0,result=0;
        char buf[BUF_SIZE]={0};
        while(1){
                while((read(storage.cfd,&buf[i],1)>0)&&buf[i]!='\n'){
                        i++;
                }
                i++;
                buf[i]='\0';
                if(buf[0]=='Q'){
                        approval(storage.cfd);
                        return 1;
                }else if(buf[0]=='D'){
                        ds=dataConnection(storage);
                        if(ds==-1){
                                return -1;
                        }
                        i=0;
                }else if(buf[0]=='L'){
                        result=lsServerOperation(storage,ds);
                        if(result!=1){
                                approval(storage.cfd);
                        }
                        i=0;
                }else if(buf[0]=='C'){
                        result=rcdServerOperation(storage,buf+1);
                        if(result!=1){
                                approval(storage.cfd);
                        }
                        i=0;
                }else if(buf[0]=='G'){
                        result=getServerOperation(storage,buf+1,ds);
                        i=0;
                }else if(buf[0]=='P'){
                        result=putServerOperation(storage,buf+1,ds);
                        if(result==-1){
                                return -1;
                        }
                        i=0;
                }
        }
                return 0;
}

int putServerOperation(struct maintenance storage, char *path, int ds){
        int errNum=0,i=strlen(path),numRead=0;
        char clientMessage[CLIENTNUM]={0};
        char buf[DATANUM]={0};
        path[i-1]='\0';
        if(storage.debugFlag){
                printf("Put command received. Checking if server has access to the file already\n");
        }
        if(access(path,F_OK)==0){
                strcpy(clientMessage,"EFile already exists on the server\n");
                write(storage.cfd,clientMessage,strlen(clientMessage));
                close(ds);
                return 1;
        }
        mode_t mode=O_CREAT|O_EXCL|O_WRONLY;
        mode_t mode2=S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;

        if(storage.debugFlag){
                printf("Opening/creating file\n");
        }
        int fd=open(path,mode,mode2);
        if (fd==-1){
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                write(storage.cfd,clientMessage,strlen(clientMessage));
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Sending positive acknowledgement to client\n");
        }
        approval(storage.cfd);
        if(storage.debugFlag){
                printf("Transmitting file information...\n");
        }
        while((numRead=read(ds,&buf,DATANUM))>0) {
                if(write(fd,&buf,numRead)==-1){
                        errNum=errno;
                        sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                        write(2,clientMessage,strlen(clientMessage));
                        close(ds);
                        return -1;
                }
        }
        if(numRead==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                write(2,clientMessage,strlen(clientMessage));
                close(ds);
                return -1;
        }
        if(close(fd)==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                write(2,clientMessage,strlen(clientMessage));
                close(ds);
                return -1;
        }
        if(close(ds)==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                close(ds);
                return -1;
        }
        if(storage.debugFlag){
                printf("Transmittion of file information complete. File is now local.\n");
        }
        return 0;
}

int getServerOperation(struct maintenance storage, char *path, int ds){
        int errNum=0,numRead=0;
        int i=strlen(path);
        char buf[DATANUM]={0};
        char clientMessage[CLIENTNUM]={0};
        path[i-1]='\0';
        if(storage.debugFlag){
                printf("Inside get operation. Seeing if I have access to the file\n");
        }
        if(access(path,F_OK)==-1){
                errNum=errno;
                sprintf(buf,"E%s\n",strerror(errNum));
                write(storage.cfd,buf,strlen(buf));
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Opening file for reading\n");
        }

        mode_t mode=S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
        int fd=open(path,O_RDONLY,mode);
        if(fd==-1){
                errNum=errno;
                sprintf(buf,"E%s\n",strerror(errNum));
                write(storage.cfd,buf,DATANUM);
                close(ds);
                return 1;
        }
        if(storage.debugFlag){
                printf("Sending positive acknowledgement\n");
                printf("Transmitting file information...\n");
        }
        approval(storage.cfd);
        while((numRead=read(fd,&buf,DATANUM))>0){
                if(write(ds,&buf,numRead)==-1){
                        errNum=errno;
                        sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                        write(2,clientMessage,strlen(clientMessage));
                        close(ds);
                        return -1;
                }
        }
        if(numRead==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                write(2,clientMessage,strlen(clientMessage));
                close(ds);
                return -1;
        }
        if(close(fd)==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: internal server error: %s\n",strerror(errNum));
                write(2,clientMessage,strlen(clientMessage));
                close(ds);
                return -1;
        }

        if(close(ds)==-1){
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                close(ds);
                return -1;
        }
        if(storage.debugFlag){
                printf("Done getting file\n");
        }
        return 0;
}

int rcdServerOperation(struct maintenance storage,char *path){
        int errNum=0,i=strlen(path);
        char clientMessage[CLIENTNUM];
        struct stat statbuf;
        path[i-1]='\0';
        if(storage.debugFlag){
                printf("Inside rcd operation\n");
                printf("Path is %s\n",path);
        }
        if(lstat(path,&statbuf)==-1){
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                write(storage.cfd,clientMessage,strlen(clientMessage));
                return 1;
        }
        if(storage.debugFlag){
                printf("Checking if path is a directory\n");
        }
        if(!S_ISDIR(statbuf.st_mode)){
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                write(storage.cfd,clientMessage,strlen(clientMessage));
                return 1;
        }
        if(storage.debugFlag){
                printf("Checking access to the pathway\n");
        }
        if(access(path, R_OK)==-1){
                printf("Am I in access?\n");
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                write(storage.cfd,clientMessage,strlen(clientMessage));
                return 1;
        }
        if(storage.debugFlag){
                printf("Changing directory into %s\n",path);
        }
        if(chdir(path)==-1){
                printf("If directory change actually failed\n");
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                write(storage.cfd,clientMessage,strlen(clientMessage));
                return 1;
        }
        if(storage.debugFlag){
                printf("Successfully changed into %s\n",path);
        }
        return 0;
}

int approval(int cfd){
        int errNum=0;
        char clientMessage[CLIENTNUM]={0};
        if(write(cfd,"A\n",2)==-1){
                errNum=errno;
                sprintf(clientMessage,"Error: Eternal Server error: %s\n",strerror(errNum));
                write(2,clientMessage,strlen(clientMessage));
                exit(1);
        }
        return 0;
}

int lsServerOperation(struct maintenance storage,int ds){
        int errNum=0,pid=0,numRead=0;
        int fd[2];
        char buf[DATANUM]={0},clientMessage[CLIENTNUM]={0};
        if(storage.debugFlag){
                printf("Inside rls operation\n");
        }
        pipe(fd);
        pid=fork();
        if(pid==0){
                if(storage.debugFlag){
                        printf("Child process %d getting ready to execute ls -la\n",getpid());
                }
                close(fd[rdr]);
                close(1);
                dup(fd[wtr]);
                close(fd[wtr]);
                if(execlp("ls", "ls", "-la",NULL)==-1){
                        errNum=errno;
                        sprintf(clientMessage,"Error: Eternal Server error: %s\n",strerror(errNum));
                        write(2,clientMessage,strlen(clientMessage));
                        return -1;
                }
        }else{
                if(storage.debugFlag){
                        printf("Transmitting data in parent process %d\n",getpid());
                }
                close(fd[wtr]);
                while((numRead=read(fd[rdr],&buf,DATANUM))>0){
                        if(write(ds,&buf,DATANUM)==-1){
                                errNum=errno;
                                sprintf(clientMessage,"Error: Eternal Server error: %s\n",strerror(errNum));
                                write(2,clientMessage,strlen(clientMessage));
                                return -1;
                        }
                }
                close(fd[rdr]);
        }
        if(close(ds)==-1){
                errNum=errno;
                sprintf(clientMessage,"E%s\n",strerror(errNum));
                close(ds);
                return -1;
        }
        if(storage.debugFlag){
                printf("Rls operation is complete\n");
        }
        return 0;
}

int dataConnection(struct maintenance storage){
        if(storage.debugFlag){
                printf("Establishing a data connection\n");
        }
        struct sockaddr_in addr;
        int ds=0,errNum=0;
        char buf[BUF_SIZE]={0};
        memset(&addr,0,sizeof(struct sockaddr_in));
        addr.sin_family=AF_INET;
        addr.sin_port=htons(0);
        int len=sizeof(struct sockaddr_in);

        if((ds=socket(AF_INET,SOCK_STREAM,0))==-1){
                errNum=errno;
                printf("Failed to create data socket\n");
                return -1;
        }
        if(storage.debugFlag){
                printf("Data socket created with descriptor 5\n");
        }

        if((bind(ds,(struct sockaddr *)&addr,sizeof(addr)))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                return -1;
        }
        if((getsockname(ds,(struct sockaddr *)&addr,&len))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                return -1;
        }
        snprintf(buf,sizeof(buf),"A%d\n",ntohs(addr.sin_port));
        write(storage.cfd, buf, strlen(buf));

        if((listen(ds, BACKLOG))==-1){
                errNum=errno;
                fprintf(stdout,"Error: %s\n",strerror(errNum));
                return -1;
        }
        if(storage.debugFlag){
                printf("Listening on data socket\n");
        }

        if(storage.debugFlag){
                printf("Sending acknowledgement -> A%d\n",ntohs(addr.sin_port));
        }

        int dsfd=accept(ds, (struct sockaddr *)&addr, &len);
        if (dsfd==-1) {
                fprintf(stdout, "Error: %s\n", strerror(errno));
                close(ds);
                return -1;
        }
        close(ds);
        return dsfd;
}
