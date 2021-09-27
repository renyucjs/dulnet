/*****************************************************************************
* dulnet.c: Copyright 2021 (c) renyucjs
* Desc:           
* 
* Author: Kimson (renyucjs@gmail.com)
* Date: 2021-09-25
* Notes: 
* 
* ---------------------------------------------------------------------------
* LastEditors: 
* LastEditTime: 
*****************************************************************************/
/*------------------------------- Includes ----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

/*------------------- Global Definitions and Declarations -------------------*/

/*---------------------- Constant / Macro Definitions -----------------------*/

/*----------------------- Type Declarations ---------------------------------*/

/*----------------------- Variable Declarations -----------------------------*/

/*----------------------- Function Prototype --------------------------------*/

/*----------------------- Function Implement --------------------------------*/
static int tcp_listen(char *ip, int port, char *dev_name)
{
    int err;
    struct sockaddr_in addr;

    int skt = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    err = setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    if (err != 0) 
    {
        printf ("could not set SO_REUSEPORT (%s)\n", strerror(errno));
        exit (EXIT_FAILURE);
    }

    if (dev_name)
    {
        // struct ifreq netif;
        // memset(&netif, 0, sizeof(netif));
        // strncpy(netif.ifr_ifrn.ifrn_name, dev_name, IFNAMSIZ-1);
        // err = setsockopt(skt, SOL_SOCKET, SO_BINDTODEVICE, &netif, sizeof(netif));
        err = setsockopt(skt, SOL_SOCKET, SO_BINDTODEVICE, dev_name, strlen(dev_name));
        if (err != 0) 
        {
            printf ("could not set SO_BINDTODEVICE (%s)\n", strerror(errno));
            exit (EXIT_FAILURE);
        }
        printf("bind to %s, rtn %d\n", dev_name, err);
    }
    
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    if(ip)
    {
        addr.sin_addr.s_addr = inet_addr(ip);
    }
    addr.sin_port = htons(port);

    err = bind(skt, (struct sockaddr *)&addr, sizeof(addr));
    assert(!err);

    printf("Listen at[%s] %s:%d...\n", dev_name?dev_name:" ", ip, port);

    err = listen(skt, 8);
    assert(!err);

    return skt;
}

static int echo(int fd)
{
    char buff[1024];
    int len = recv(fd, buff, sizeof(buff)-1, 0);
    if(len)
    {
        send(fd, buff, len, 0);
    }
    buff[len] = 0;
    printf("rec: %s\r\n", buff);
    return 0;
}

int main(int argc, char *argv[])
{
    char *localip = NULL;
    char netdev[IFNAMSIZ] = "eth0\0";
    int port = 12345;

    const char *opt = "hl:p:i:";
    int ch;
    while ((ch = getopt(argc, argv, opt)) != -1)
    {
        switch (ch)
        {
        case 'l':  /* local ip: -l 127.0.0.1 */
            localip = optarg;
            break;
        case 'p': /* port: -p 12345 */
            port = atoi(optarg);
            break;
        case 'i':  /* interface: -i eth0 */
            strncpy(netdev, optarg, IFNAMSIZ-1);
            break;
        case 'h':  /* Print Usage */
            printf("Usage: dulnet [-l local_ip, default:any] [-p port, defalut:12345] [-i interface, default:eth0]\r\n");
            printf("Example: dulnet -l 127.0.0.1 -p 12345 -i eth0 \n");
            return 0;
        default:
            printf("Unknown option: %c\n",(char)optopt);
            printf("Usage: dulnet [-l local_ip, default:any] [-p port, defalut:12345] [-i interface, default:eth0]\r\n");
            printf("Example: dulnet -l 127.0.0.1 -p 12345 -i eth0 \n");
            return -1;
        }
    }
    
    int sfd = tcp_listen(localip, port, netdev);
    int epfd = epoll_create (100);
    struct epoll_event wait_evts[32];
    struct epoll_event epevt;

    epevt.events = EPOLLIN;
    epevt.data.fd = sfd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD , sfd, &epevt) != 0)
    {
        printf ("epoll_ctl fail (%s)\n", strerror(errno));
        exit (EXIT_FAILURE);
    }

    while (1)
    {
        int nfds = epoll_wait(epfd, wait_evts, sizeof(wait_evts)/sizeof(wait_evts[0]), -1);
        if(nfds < 0)
        {
            printf ("epoll_wait fail (%s)\n", strerror(errno));
            exit (EXIT_FAILURE);
        }
        
        for(int i = 0; i < nfds; i++)
        {
            if((wait_evts[i].events & EPOLLERR) || (wait_evts[i].events & EPOLLHUP) || !(wait_evts[i].events & EPOLLIN))
            {
                close(wait_evts[i].data.fd);
                printf ("epoll_wait fail at fd: %d\n", wait_evts[i].data.fd);
                exit (EXIT_FAILURE);
            }

            if(wait_evts[i].data.fd == sfd)//new accept
            {
                int fd = accept(sfd, NULL, NULL);
                if(fd < 0)
                {
                    printf ("accept fail (%s)\n", strerror(errno));
                    exit (EXIT_FAILURE);
                }
                //append to epoll
                epevt.data.fd = fd;
                epevt.events = EPOLLIN|EPOLLET;
                if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &epevt) != 0)
                {
                    printf ("epoll_ctl fail (%s)\n", strerror(errno));
                    exit (EXIT_FAILURE);
                }
                printf("accept[%s]: %d\r\n", netdev, fd);
            }
            else
            {
                echo(wait_evts[i].data.fd);
            }
        }
    }
    
    close(sfd);
    close(epfd);
}

/*---------------------------------------------------------------------------*/
