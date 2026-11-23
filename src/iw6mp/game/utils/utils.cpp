#include "utils.hpp"

#include <libpsutil.h>

namespace Game
{
    void Com_vsnprintf( char *dest, int size, const char *fmt, char *vargs )
    {
        static libpsutil::symbol<void( char *dest, int size, const char *fmt, char *vargs )> Com_vsnprintf{ 0x38CF48 };
        Com_vsnprintf( dest, size, fmt, vargs );
    }

    int CG_GetClientNum( int localClientNum )
    {
        libpsutil::symbol<int( int localClientNum )> CG_GetClientNum{ 0x586444 };
        return CG_GetClientNum( localClientNum );
    }

    void Cbuf_ExecuteBuffer( int localClientNum, int controllerIndex, const char *buffer )
    {
        libpsutil::symbol<void( int localClientNum, int controllerIndex, const char *buffer )> Cbuf_ExecuteBuffer{ 0x2B24C8 };
        Cbuf_ExecuteBuffer( localClientNum, controllerIndex, buffer );
    }

    void Cbuf_InsertText( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> Cbuf_InsertText{ 0x2B1DF4 };
        Cbuf_InsertText( localClientNum, text );
    }

    void Cbuf_AddText( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> Cbuf_AddText{ 0x2B1C14 };
        Cbuf_AddText( localClientNum, text );
    }

    char* va( const char* format, ... )
    {
        libpsutil::symbol<char*( const char* format, ... )> va{ 0x38DC30 };

        va_list args;
        va_start( args, format );
        
        char* result = va( format, args );
        
        va_end( args );

        return result;
    }

    void CG_GameMessage( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> CG_GameMessage{ 0x5BB454 };
        CG_GameMessage( localClientNum, text );
    }

    void CG_GameMessageBold( int localClientNum, const char *text )
    {
        libpsutil::symbol<void( int localClientNum, const char *text )> CG_GameMessageBold{ 0x5A9F14 };
        CG_GameMessageBold( localClientNum, text );
    }

    int _sscanf( const char *text, const char* fmt, ... )
    {
        libpsutil::symbol<int( const char *text, const char* fmt, ... )> _sscanf{ 0x7763D4 };

        va_list args;
        va_start( args, fmt );

        int result = _sscanf( text, fmt, args );

        va_end( args );

        return result;
    }

    int Key_StringToKeynum( const char *str )
    {
        libpsutil::symbol<int( const char *str )> Key_StringToKeynum{ 0x18EEF8 };
        return Key_StringToKeynum( str );
    }
}