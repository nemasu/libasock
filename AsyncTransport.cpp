#include "AsyncInterface.h"
#include "AsyncTransport.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>

#define EPOLL_TIMEOUT_MS 1000

using std::cerr;
using std::endl;

AsyncTransport::AsyncTransport( PacketParser &pp ) 
    : packetParser(pp) {
    epollSendFD  = epoll_create( 1 );
    bufferQueue.setEpollFd(epollSendFD);
}

AsyncTransport::~AsyncTransport() {
    if( isRunning ) {
        stop();
    }
    close(epollSendFD);
}

void
AsyncTransport::closeFd( int fd ) {
    lock_guard<mutex> lock( closeMutex );
    close( fd );
    bufferQueue.closeFd(fd);
}

Packet*
AsyncTransport::getPacket() {
    packetQueue.wait();
    return packetQueue.pop();
}

void
AsyncTransport::sendPacket( Packet * pkt ) {
    
    if (pkt->type == DISCONNECT) {
        closeFd( pkt->fd );
        return;
    }
    
    unsigned int length = 0;
    char *buffer = packetParser.serialize( pkt, &length );
    
    if( !buffer ) {
        cerr << "AsyncTransport::sendPacket - serialize failed" << endl;
        return;
    }

    if (!isServer) {
        pkt->fd = fd;
    }

    //Try to send packet right away, if it blocks, do epoll stuff
    int ret = 0;
    int sent = 0;
    while(1) {
        ret = handleSend( pkt->fd, buffer+sent, length-sent, MSG_NOSIGNAL );
        if( ret == (int) length-sent ) {
            break; //All sent OK
        } else if( (ret == -1) && (errno == EWOULDBLOCK || errno == EAGAIN ) ) {
            bufferQueue.put( pkt->fd, buffer+sent, length-sent ); //Would block, do epoll stuff
            break;
        }
        sent += ret;
    }

    delete [] buffer;
    //Client bound packets die here :(
    delete pkt;
}

bool
AsyncTransport::init( string addr, int port ) {
    isServer = false;

    struct sockaddr_in server;
    struct hostent *hp;

    fd = socket (AF_INET, SOCK_STREAM, 0);
    
    server.sin_family = AF_INET;
    hp = gethostbyname(addr.c_str());
    bcopy ( hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
    server.sin_port = htons(port);

    int one = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(int) );
    setsockopt( fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(int) );
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int) );

    int flags = fcntl( fd, F_GETFL, 0 );
    fcntl( fd, F_SETFL, flags | O_NONBLOCK );
    
    int ret = connect(fd, (const sockaddr *) &server, sizeof(server));

    if( !onAfterConnect( fd ) ) {
        exit(-5);
    }

    return ret == 0 ? true : false;
}

bool
AsyncTransport::init( int port ) {
    isServer = true;
    struct sockaddr_in server;
     
    //Create socket
    fd = socket(AF_INET , SOCK_STREAM , 0);
    if ( fd == -1 )
    {
        return false;
    }
    int one = 1;
    setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(int) );
    setsockopt( fd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(int) );
    setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int) );

    int flags = fcntl( fd, F_GETFL, 0 );
    fcntl( fd, F_SETFL, flags | O_NONBLOCK );

    //Fill sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port );
     
    //Bind
    if( bind( fd, (sockaddr *)&server , sizeof(server) ) < 0 )
    {
        return false;
    }
     
    //Listen
    listen( fd, 1000 );
    return true;
}

void
AsyncTransport::start() {
    isRunning = true;
    recvThread = thread( receiveData, std::ref(*this) );
    sendThread = thread( sendData,    std::ref(*this) );

    recvThread.detach();
    sendThread.detach();
}

void
AsyncTransport::stop() {
    isRunning = false;

    for ( auto cd : pendingData ) {
        delete cd;
    }

    pendingData.clear();
    
    //Need to push a NULL packet to have getPacket return
    packetQueue.push(NULL);
}

void
AsyncTransport::receiveData( AsyncTransport &serverTransport ) {
    PacketParser &packetParser = serverTransport.packetParser;
    PacketQueue  &packetQueue  = serverTransport.packetQueue;
    set<ConnectionData*> &pendingData = serverTransport.pendingData;

    static const unsigned int MAX_EVENTS = 1000;
    epoll_event ev = { 0 }; 
    epoll_event events[MAX_EVENTS];
    int nfds, epollFD, connFD, recvCount;
    
    epollFD = epoll_create( 1 );

    if( epollFD == -1 ) {
        exit(-1);
    }

    bool isServer = serverTransport.isServer;
    int serverFD = 0;
    if ( isServer ) {
        serverFD = serverTransport.fd;
        ev.events = EPOLLIN;
        ev.data.fd = serverFD;
        if( epoll_ctl(epollFD, EPOLL_CTL_ADD, serverFD, &ev ) == -1 ) {
            exit(-2);
        }
    }

    while( serverTransport.isRunning ) {
        nfds = epoll_wait( epollFD, events, MAX_EVENTS, EPOLL_TIMEOUT_MS );
        if( nfds == -1 ) {
            exit(-3);
        }
        int flags;
        int one = 1;
        for( int n = 0; n < nfds; ++n ) {
            if( isServer && events[n].data.fd == serverFD ) {
                connFD = accept( serverFD, NULL, NULL );

                if( connFD == -1 ) {
                    continue;
                }

                setsockopt( connFD, IPPROTO_TCP, TCP_NODELAY,  &one, sizeof(int) );
                setsockopt( connFD, SOL_SOCKET,  SO_KEEPALIVE, &one, sizeof(int) );
                setsockopt( connFD, SOL_SOCKET,  SO_REUSEADDR, &one, sizeof(int) );

                flags = fcntl( connFD, F_GETFL, 0 );
                fcntl( connFD, F_SETFL, flags | O_NONBLOCK );

                ev.events = EPOLLIN | EPOLLET;
                ConnectionData *cd = new ConnectionData;
                pendingData.insert(cd);
                ev.data.ptr = (void *) cd;
                cd->fd = connFD;
                cd->bufferSize = 0;
                if( epoll_ctl( epollFD, EPOLL_CTL_ADD, connFD, &ev ) == -1 ) {
                    exit(-4);
                }

                if( !serverTransport.onAfterAccept( connFD ) ) {
                    exit(-5);
                }
                
                Packet *packet = new Packet();
                packet->type = PacketType::CONNECT;
                packet->fd = connFD;
                packetQueue.push( packet );
            } else {
                ConnectionData *cd = (ConnectionData *) events[n].data.ptr;
                
                while(1) {
                    recvCount = serverTransport.handleReceive( *cd );
                
                    if      ( (recvCount == -1 && (errno == EAGAIN || errno == EWOULDBLOCK )) ) {
                        break;
                    }
                    else if ( recvCount == 0 
                         ||   recvCount == -1
                         ||   cd->bufferSize > MAX_PACKET_SIZE ) {
                        
                        //Make sure it hasn't been deleted already
                        if( pendingData.count(cd) > 0 ) {
                            serverTransport.closeFd( cd->fd );
                            pendingData.erase(cd);
                            delete cd;
                        }
                        break;
                    }
                    
                    cd->bufferSize += recvCount;

                    while (1) {
                        unsigned int bufferUsed = 0;
                        Packet *newPacket = packetParser.deserialize( cd->buffer, cd->bufferSize, &bufferUsed );

                        if( newPacket == NULL && bufferUsed == 0 ) {
                            //Deserialize failed, wait for more data.
                            break;
                        } else if ( (long) newPacket == -1 ) {
                            //Something very bad happened, clear buffer and try and recover.
                            cd->bufferSize = 0;
                            break;
                        }

                        //Got a packet.
                        if (   MAX_PACKET_SIZE - cd->bufferSize > MAX_PACKET_SIZE
                            || bufferUsed + MAX_PACKET_SIZE - cd->bufferSize > MAX_PACKET_SIZE ) {
                            
                            cerr << "TCPTransport memmove dest error" << endl;
                            if ( newPacket != NULL ) {
                                delete newPacket;
                            }
                            cd->bufferSize = 0;
                            break;
                        }
            
                        //Assign socket file descriptor
                        if( newPacket != NULL ) {
                            newPacket->fd = cd->fd;
                            packetQueue.push( newPacket );
                        }

                        memmove( cd->buffer, cd->buffer + bufferUsed, MAX_PACKET_SIZE - cd->bufferSize );

                        cd->bufferSize -= bufferUsed;

                        if( cd->bufferSize == 0 ) {
                            break;
                        }
                    }
                }
            }
        }
    }
}

void
AsyncTransport::sendData( AsyncTransport &serverTransport ) {
    static const unsigned int MAX_EVENTS = 1000;
    epoll_event events[MAX_EVENTS];
    int nfds;

    int epollSendFD = serverTransport.epollSendFD;
    BufferQueue &bufferQueue = serverTransport.bufferQueue;
    
    if( epollSendFD == -1 ) {
        exit(-10);
    }
    
    while( serverTransport.isRunning ) {
        nfds = epoll_wait( epollSendFD, events, MAX_EVENTS, EPOLL_TIMEOUT_MS );
        if( nfds == -1 ) {
            exit(-20);
        }
        
        int sent = -1;
        for( int n = 0; n < nfds; ++n ) {
            int fd = events[n].data.fd;
            int length = 0;
            char *buffer = 0;

            while(1) {
                bufferQueue.get( fd, buffer, &length );
                if( length == 0 ) {
                    break;
                }
                sent = serverTransport.handleSend(fd, buffer, length, 0);
                if( (sent == -1) && (errno == EWOULDBLOCK || errno == EAGAIN ) ) {
                    break;
                } else if( sent == -1 || sent == 0 ) {
                    serverTransport.closeFd(fd);
                }
                bufferQueue.updateUsed( fd, sent );
            }
        }
    }
}

int
AsyncTransport::handleReceive( ConnectionData &cd ) {
    return recv( cd.fd, cd.buffer + cd.bufferSize, MAX_PACKET_SIZE - cd.bufferSize, MSG_NOSIGNAL );
}

int
AsyncTransport::handleSend( int fd, char *buffer, int length, int flags ) {
    return send( fd, buffer, length, flags );
}

bool
AsyncTransport::onAfterAccept( int fd ) { 
     return true;
}

bool
AsyncTransport::onAfterConnect( int fd ) { 
     return true;
}
