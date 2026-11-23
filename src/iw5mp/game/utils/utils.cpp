#include "utils.hpp"

#include <libpsutil.h>

namespace Game
{
    void Com_vsnprintf( char *dest, int size, const char *fmt, char *vargs )
    {
        static libpsutil::symbol<void( char *dest, int size, const char *fmt, char *vargs )> Com_vsnprintf{ 0x49286C };
        Com_vsnprintf( dest, size, fmt, vargs );
    }

    int CG_GetClientNum( int localClientNum )
    {
        libpsutil::symbol<int( int localClientNum )> CG_GetClientNum{ 0x7BFC4 };
        return CG_GetClientNum( localClientNum );
    }

    void Cbuf_ExecuteBuffer( int localClientNum, int controllerIndex, const char *buffer )
    {
        libpsutil::symbol<void( int localClientNum, int controllerIndex, const char *buffer )> Cbuf_ExecuteBuffer{ 0x1DBAEC };
        Cbuf_ExecuteBuffer( localClientNum, controllerIndex, buffer );
    }

    void Cbuf_InsertText( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> Cbuf_InsertText{ 0x1DB424 };
        Cbuf_InsertText( localClientNum, text );
    }

    void Cbuf_AddText( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> Cbuf_AddText{ 0x1DB240 };
        Cbuf_AddText( localClientNum, text );
    }

    char* va( const char* format, ... )
    {
        libpsutil::symbol<char*( const char* format, ... )> va{ 0x299490 };

        va_list args;
        va_start( args, format );
        
        char* result = va( format, args );
        
        va_end( args );

        return result;
    }

    void CG_GameMessage( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> CG_GameMessage{ 0x7A588 };
        CG_GameMessage( localClientNum, text );
    }

    void CG_GameMessageBold( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> CG_GameMessageBold{ 0x7A5C8 };
        CG_GameMessageBold( localClientNum, text );
    }

    int _sscanf( const char *text, const char* fmt, ... )
    {
        libpsutil::symbol<int( const char *text, const char* fmt, ... )> _sscanf{ 0x492488 };

        va_list args;
        va_start( args, fmt );

        int result = _sscanf( text, fmt, args );

        va_end( args );

        return result;
    }

    int Key_StringToKeynum( const char *str )
    {
        libpsutil::symbol<int( const char *str )> Key_StringToKeynum{ 0xD1D18 };
        return Key_StringToKeynum( str );
    }
}