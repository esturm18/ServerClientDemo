
all: myftp myftpserve
myftp: myftp.c
        gcc myftp.c myftp.h -o myftp
myftpserve: myftpserve.c
        gcc myftpserve.c myftp.h -o myftpserve
clean:
        rm  myftp myftpserve

