#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 10000 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main(int argc, char *argv[])
{
    sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if(sock_fd<0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

//check the format
    int check=3;
    int length=strlen(argv[1]);
    char option_pid[50];
    strcpy(option_pid,argv[1]);
    if(argc>2) {
        printf("Format isn't correct!!\n");
        close(sock_fd);
        return -1;
    } else if(argc==1) {
        strcpy(NLMSG_DATA(nlh), "-c1");
    } else if(argc==2) {
        if(length==2) {
            if(option_pid[1]=='c'||option_pid[1]=='C')
                strcpy(NLMSG_DATA(nlh), "-c1");
            else if(option_pid[1]=='s'||option_pid[1]=='S') {
                close(sock_fd);
                printf("Lack of process id\n");
                return -1;
            } else if(option_pid[1]=='p'||option_pid[1]=='P')
                strcpy(NLMSG_DATA(nlh), "-p0");
            else {
                printf("no such option!\n");
                close(sock_fd);
                return -1;
            }
        } else if(length>8) {
            printf("the format of pid os too big!\n");
            close(sock_fd);
            return -1;
        } else {
            if(argv[1][0]=='-'&&(argv[1][1]=='c'||argv[1][1]=='C'||argv[1][1]=='S'||argv[1][1]=='s'||argv[1][1]=='P'||argv[1][1]=='p')) {
                for(check=3; check<length+1; check++) {
                    if(isalpha(option_pid[check-1])!=0) {
                        printf("the format of pid is unavailable!\n");
                        close(sock_fd);
                        return -1;
                    }
                    strcpy(NLMSG_DATA(nlh), option_pid);
                }
            } else {
                printf("your option is unavailable!\n");
                close(sock_fd);
                return -1;
            }
        }


    }


    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;


    sendmsg(sock_fd,&msg,0);

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("%s\n", (char *)NLMSG_DATA(nlh));

    close(sock_fd);
}