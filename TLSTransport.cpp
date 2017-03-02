#include "TLSTransport.h"

#ifndef DEBUG
#define DEBUG 0
#endif

void
ShowSSLErrors() {
    ERR_print_errors_fp(stderr);
}


TLSTransport::TLSTransport( PacketParser &packetParser, string certificateFile, string privateKeyFile ) 
    : AsyncTransport( packetParser ), certificateFile( certificateFile ), privateKeyFile( privateKeyFile ) {
}

TLSTransport::~TLSTransport() {
    SSL_CTX_free(ctx);
    EVP_cleanup();
}

bool
TLSTransport::init( int port ) {

    SSL_load_error_strings();    
    OpenSSL_add_ssl_algorithms();

    const SSL_METHOD *method = SSLv23_server_method();
    ctx = SSL_CTX_new( method );
    if( !ctx ) {
        std::cerr << "Unable to create SSL context" << std::endl;
        ShowSSLErrors();
        return false;
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, certificateFile.c_str(), SSL_FILETYPE_PEM) < 0) {
        ShowSSLErrors();
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, privateKeyFile.c_str(), SSL_FILETYPE_PEM) < 0 ) {
        ShowSSLErrors();
        return false;
    }

    return AsyncTransport::init( port );
}

bool
TLSTransport::onAfterAccept( int fd ) {
    SSL *ssl = SSL_new( ctx );
    if( NULL == ssl ) {
        ShowSSLErrors();
        std::cerr << "SSL new failed." << std::endl;
    }
    
    fdToSSL[fd] = ssl;
    
    if ( 0 == SSL_set_fd( ssl, fd ) ) {
        ShowSSLErrors();
        std::cerr << "SSL set fd failed." << std::endl;
    }

    if( SSL_accept(ssl) >= 0 ) {
        ShowSSLErrors();
        std::cerr << "SSL accept failed" << std::endl;
    }

    //TODO this is non-blocking, it may fail, but it might be okay cause it will re-do handshake on read or write anyways.
    if ( SSL_do_handshake( ssl ) >= 0 ) {
        ShowSSLErrors();
        std::cerr << "SSL do handshake failed" << std::endl;
    }
    return true;
}

int
TLSTransport::handleReceive( ConnectionData &cd ) {
    SSL *ssl = fdToSSL[cd.fd];
    int ret = SSL_read( ssl, cd.buffer + cd.bufferSize, MAX_PACKET_SIZE - cd.bufferSize );
    if( ret > 0 ) {
        //ShowSSLErrors();
        //std::cerr << "handleReceive > 0 " << ret << std::endl;
        return ret;
    } else if ( ret == 0 ) {
        //ShowSSLErrors();
        //std::cerr << "handleReceive == 0 " << ret << std::endl;
        //TODO potential shutdown, just close it for now
        cleanupSSL(ssl);
        return 0;
    } else {
        int err = SSL_get_error( ssl, ret );
        if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ) {
            errno = EAGAIN;
            return -1;
        }
        if(DEBUG) {
            ShowSSLErrors();
            std::cerr << "libasock: TLSTransort::handleReceive else " << ret << ", err: " << err << std::endl;
        }
        //Close on err
        cleanupSSL(ssl);
        return 0;
    }
}

int
TLSTransport::handleSend( int fd, char *buffer, int length, int flags ) {
    SSL *ssl = fdToSSL[fd];
    int ret = SSL_write( ssl, buffer, length );

    if( ret > 0 ) {
        //ShowSSLErrors();
        //std::cerr << "handleSend > 0 " << ret << std::endl;
        return ret;
    } else if ( ret == 0 ) {
        //ShowSSLErrors();
        //std::cerr << "handleSend == 0 " << ret << std::endl;
        //TODO potential shutdown, just close it for now
        cleanupSSL(ssl);
        return 0;
    } else {

        int err = SSL_get_error( ssl, ret );
        if( err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE ) {
            errno = EAGAIN;
            return -1;
        }
        if(DEBUG) {
            ShowSSLErrors();
            std::cerr << "libasock: TLSTransort::handleSend else " << ret << ", err: " << err << std::endl;
        }
        //Close on err
        cleanupSSL(ssl);
        return 0;
    }
}

void
TLSTransport::cleanupSSL( SSL* ssl ) {
    SSL_free(ssl);
}
