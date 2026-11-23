#include "webman.hpp"

#include <libpsutil.h>
#include <cell/http.h>
#include <sys/socket.h>

void SendWebManRequest( const char* command ) {
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0x7F000001;
    sin.sin_port = htons( 80 );

    int socket_connection = socket( AF_INET, SOCK_STREAM, 0 );
    if ( socket_connection < 0 || connect( socket_connection, ( struct sockaddr* )&sin, sizeof( sin )) < 0)
    {
        socket_connection = -1;
    }

    struct timeval time_val;
    time_val.tv_usec = 0;
    time_val.tv_sec = 3;

    setsockopt( socket_connection, SOL_SOCKET, SO_SNDTIMEO, &time_val, sizeof( time_val ) );
    setsockopt( socket_connection, SOL_SOCKET, SO_RCVTIMEO, &time_val, sizeof( time_val ) );

    if ( socket_connection < 0 )
    {
        return;
    }

    char webman_command[ 1048 ];
    int length = _sys_sprintf( webman_command, "GET %s HTTP/1.0\r\n", command );
    send( socket_connection, webman_command, length, 0 );

    if ( socket_connection == -1 )
    {
        return;
    }

    shutdown( socket_connection, SHUT_RDWR );
    socketclose( socket_connection );
    socket_connection = -1;
}
      
void FixSpaces( const char* string, char* output )
{
    int index = 0;

    for( size_t i = 0; string[ i ] != '\0'; i++ )
    {
        if ( string[ i ] != ' ' )
        {
            output[ index++ ] = string[ i ];
        }
        else
        {
            output[ index++ ] = '%';
            output[ index++ ] = '2';
            output[ index++ ] = '0';
        }
    }

    output[ index ] = '\0';
}

void WebmanNotify( const char* msg )
{
    char buffer[ 512 ];
    char escaped[ 512 ];

    FixSpaces( msg, escaped );

    sprintf( buffer, "/notify.ps3mapi?msg=%s", escaped );

    SendWebManRequest( buffer );
}