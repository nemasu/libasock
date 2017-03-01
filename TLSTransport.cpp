#include "TLSTransport.h"

bool
TLSTransport::init( string serverHost, int port ) {

    SSL_load_error_strings();    
    OpenSSL_add_ssl_algorithms();

    const SSL_METHOD *method = SSLv23_server_method();
    ctx = SSL_CTX_new( method );
    if( !ctx ) {
        std::cerr << "Unable to create SSL context" << std::endl;
        return false;
    }

	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, certificateFile.c_str(), SSL_FILETYPE_PEM) < 0) {
		ERR_print_errors_fp(stderr);
        return false;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, privateKeyFile.c_str(), SSL_FILETYPE_PEM) < 0 ) {
		ERR_print_errors_fp(stderr);
        return false;
	}

    return AsyncTransport::init( serverHost, port );
}

bool
TLSTransport::onAfterAccept( int fd ) {
    fdToSSL[fd] = SSL_new( ctx );
    SSL_set_fd( fdToSSL[fd], fd );
    //TODO this is non-blocking, it may fail, but it might be okay cause it will re-do handshake on read or write anyways.
    SSL_do_handshake( fdToSSL[fd] );
    return true;
}

int
TLSTransport::handleReceive( ConnectionData &cd ) {
    SSL *ssl = fdToSSL[cd.fd];
    int ret = SSL_read( ssl, cd.buffer + cd.bufferSize, MAX_PACKET_SIZE - cd.bufferSize );
    if( ret > 0 ) {
        return ret;
    } else if ( ret == 0 ) {
        //TODO potential shutdown, just close it for now
        return 0;
    } else {
        //TODO assuming this is a SSL_ERROR_WANT_READ or a SSL_ERROR_WANT_WRITE
        //TODO will this read/write get triggered again with ET epoll??
        errno = EAGAIN;
        return -1;
    }
}

int
TLSTransport::handleSend( int fd, char *buffer, int length, int flags ) {
    SSL *ssl = fdToSSL[fd];
    int ret = SSL_write( ssl, buffer, length );
    if( ret > 0 ) {
        return ret;
    } else if ( ret == 0 ) {
        //TODO potential shutdown, just close it for now
        return 0;
    } else {
        //TODO assuming this is a SSL_ERROR_WANT_READ or a SSL_ERROR_WANT_WRITE
        //TODO will this read/write get triggered again with ET epoll??
        errno = EAGAIN;
        return -1;
    }
}
