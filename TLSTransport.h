#ifndef __TLSTRANSPORT_H__
#define __TLSTRANSPORT_H__

#include "AsyncTransport.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

class TLSTransport : public AsyncTransport {
    public:
        TLSTransport( PacketParser &packetParser );
        TLSTransport( PacketParser &packetParser, string certificateFile, string privateKeyFile );
        ~TLSTransport();

        bool init( int port ) override;
        bool init( string destinationHost, int port ) override;
        int handleReceive( ConnectionData &cd ) override;
        int handleSend( int fd, char *buffer, int length, int flags ) override;
        bool onAfterAccept( int fd ) override;
        bool onAfterConnect( int fd ) override;
    private:
        string hostname;
        SSL_CTX *ctx;
        std::map< int, SSL* > fdToSSL;

        string certificateFile;
        string privateKeyFile;

        void cleanupSSL( int fd );
};

#endif
