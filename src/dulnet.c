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

#include <sys/socket.h>
#include <netinet/in.h>
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
    assert(!err);

    if (dev_name)
    {
        struct ifreq netif;
        
        memset(&netif, 0, sizeof(netif));
        strncpy(&netif.ifr_ifrn.ifrn_name, dev_name, IFNAMSIZ-1);
        err = setsockopt(skt, SOL_SOCKET, SO_BINDTODEVICE, &netif.ifr_ifrn.ifrn_name, IFNAMSIZ);
        printf("bind to %s, rtn %d\n", dev_name, err);
        assert(!err);
    }
    

    bzero(&addr, sizof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    err = bind(skt, (struct sockaddr *)&addr, sizeof(addr));
    assert(!err);

    err = listen(skt, 8);
    assert(!err);

    return 0;
}


int main(int argc, char *argv[])
{

}

/*---------------------------------------------------------------------------*/
