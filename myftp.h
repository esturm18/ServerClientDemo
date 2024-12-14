
#define RESPONSE_SIZE 255
#define MAXTERMINAL 460
#define BUF_SIZE 100
#define PORTNUM 100
#define BACKLOG 4
#define DATANUM 1024
#define CLIENTNUM 255
#define rdr 0
#define wtr 1


struct maintenance{
        char *hostName,*portNum;
        char IPaddress[100];
        int sfd,cfd,debugFlag;
};
int getServerDataConnection(struct maintenance storage);
int putServerOperation(struct maintenance storage,char *path,int ds);
int getServerOperation(struct maintenance storage,char *path,int ds);
int rcdServerOperation(struct maintenance storage,char *path);
int approval(int cfd);
int lsServerOperation(struct maintenance storage,int ds);
int dataConnection(struct maintenance storage);
int serverCommands(struct maintenance storage);

int commandLine(struct maintenance storage);
int exitOperation(struct maintenance storage);
int lsOperation(struct maintenance storage);
int rlsOperation(struct maintenance storage);
int cdOperation(char *pathname,struct maintenance storage);
int rcdOperation(char *path, struct maintenance storage);
int getOperation(char *path, struct maintenance storage);
int showOperation(char *path, struct maintenance storage);
int putOperation(char *path, struct maintenance storage);

