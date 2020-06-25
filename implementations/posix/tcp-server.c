
#include <emodbus/impl/posix/tcp-server.h>

int tcp_server_init(tcp_server_t* _srv, in_addr_t _addr, uint16_t _port)
{
    if(_srv) {
        const int yes = 1;

        FD_ZERO(&_srv->master);
        FD_ZERO(&_srv->read_fds);

        /* get the listener */
        if((_srv->listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Server-socket() error lol!");
            /*just exit lol!*/
            return -1;
        }
        printf("Server-socket() is OK...\n");
        /*"address already in use" error message */
        if(setsockopt(_srv->listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("Server-setsockopt() error lol!");
            return -1;
        }
        printf("Server-setsockopt() is OK...\n");

        /* bind */
        _srv->serveraddr.sin_family = AF_INET;
        _srv->serveraddr.sin_addr.s_addr = _addr;
        _srv->serveraddr.sin_port = htons(_port);
        memset(&(_srv->serveraddr.sin_zero), '\0', 8);

        printf("Using %s, listening at %d\n", inet_ntoa(_srv->serveraddr.sin_addr), _port);

        if(bind(_srv->listener, (struct sockaddr *)&_srv->serveraddr, sizeof(_srv->serveraddr)) == -1)
        {
            perror("Server-bind() error lol!");
            return -1;
        }
        printf("Server-bind() is OK...\n");

        /* listen */
        if(listen(_srv->listener, 10) == -1)
        {
            perror("Server-listen() error lol!");
            return -1;
        }
        printf("Server-listen() is OK...\n");

        /* add the listener to the master set */
        FD_SET(_srv->listener, &_srv->master);
        /* keep track of the biggest file descriptor */
        _srv->fdmax = _srv->listener; /* so far, it's this one*/
        return 0;
    }
    return -EINVAL;
}

int tcp_server_deinit(tcp_server_t* _srv)
{
    if(_srv) {
        if(_srv->listener >= 0) {
            close(_srv->listener);
            _srv->listener = -1;
        }
        return 0;
    }
    return -EINVAL;
}

int tcp_server_receive(tcp_server_t* _srv, int *_client_id,
                       uint8_t* _buffer, unsigned int _buf_size,
                       unsigned int _timeout_ms)
{
    if(_srv && _buffer && _buf_size) {
        int i;
        socklen_t addrlen;
        int newfd;
        int nbytes;
        int sel_res;
        struct sockaddr_in clientaddr;

        /* copy it */
        _srv->read_fds = _srv->master;

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = _timeout_ms * 1000;

        sel_res = select(_srv->fdmax+1, &_srv->read_fds, NULL, NULL, &tv);

        if(sel_res == 0) {
            printf("Server-select() timeout\n");
            return -ETIMEDOUT;
        }
        else if(sel_res < 0) {
            perror("Server-select() error lol!");
        }
        printf("Server-select() is OK...\n");

        /*run through the existing connections looking for data to be read*/
        for(i = 0; i <= _srv->fdmax; i++)
        {
            if(FD_ISSET(i, &_srv->read_fds))
            { /* we got one... */
                if(i == _srv->listener)
                {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if((newfd = accept(_srv->listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
                    {
                        perror("Server-accept() error lol!");
                    }
                    else
                    {
                        printf("Server-accept() is OK...\n");

                        FD_SET(newfd, &_srv->master); /* add to master set */
                        if(newfd > _srv->fdmax)
                        { /* keep track of the maximum */
                            _srv->fdmax = newfd;
                        }
                        printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                }
                else
                {
                    /* handle data from a client */
                    if((nbytes = (int)recv(i, _buffer, _buf_size, 0)) <= 0)
                    {
                        /* got error or connection closed by client */
                        if(nbytes == 0)
                            /* connection closed */
                            printf("Socket %d connection closed\n", i);

                        else
                            perror("recv() error lol!");

                        /* close it... */
                        close(i);
                        /* remove from master set */
                        FD_CLR(i, &_srv->master);
                    }
                    else
                    {
                        if(_client_id)
                            *_client_id = i;
                        return nbytes;
#if 0
                        /* we got some data from a client*/
                        for(j = 0; j <= _srv->fdmax; j++)
                        {
                            /* send to everyone! */
                            if(FD_ISSET(j, &_srv->master))
                            {
                                /* except the listener and ourselves */
                                if(j != _srv->listener /*&& j != i*/)
                                {
                                    printf("Sending\n");
                                    if(send(j, _buffer, (size_t)nbytes, 0) == -1)
                                        perror("send() error lol!");
                                }
                            }
                        }
#endif
                    }
                }
            }
        }
        return 0;
    }
    else {
        return -EINVAL;
    }
}

int tcp_server_send(tcp_server_t* _srv, int _client_id,
                    const uint8_t* _data, unsigned int _data_length)
{
    if(_srv && _client_id >= 0 && _data && _data_length) {
        return (int)send(_client_id, _data, _data_length, 0);
    }
    else {
        return -EINVAL;
    }
}
