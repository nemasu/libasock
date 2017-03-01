#ifndef __TLSTRANSPORT_H__
#define __TLSTRANSPORT_H__

#include "AsyncTransport.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

class TLSTransport : public AsyncTransport {
    public:
        TLSTransport( PacketParser &packetParser, string certificateFile, string privateKeyFile ) 
            : AsyncTransport( packetParser ), certificateFile( certificateFile ), privateKeyFile( privateKeyFile ) {
        }

        ~TLSTransport() {
        }

        bool init( string serverHost, int port) override;
        bool onAfterAccept( int fd ) override;
        int handleReceive( ConnectionData &cd ) override;
        int handleSend( int fd, char *buffer, int length, int flags ) override;
    private:
        SSL_CTX *ctx;
        std::map< int, SSL* > fdToSSL;

        string certificateFile;
        string privateKeyFile;
};

#endif
