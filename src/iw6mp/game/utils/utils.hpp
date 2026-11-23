#pragma once

namespace Game
{
    void Com_vsnprintf( char *dest, int size, const char *fmt, char *vargs );
    
    int CG_GetClientNum( int localClientNum );

    void Cbuf_ExecuteBuffer( int localClientNum, int controllerIndex, const char *buffer );

    void Cbuf_InsertText( int localClientNum, const char *text );

    void Cbuf_AddText( int localClientNum, const char *text );

    char* va( const char* format, ... );

    void CG_GameMessage( int localClientNum, const char *text );

    void CG_GameMessageBold( int localClientNum, const char *text );

    int Key_StringToKeynum( const char *str );

	struct Timer
	{
		private:
			int time;
			int delay;

		public:
			bool Ready( void );

			void Wait( int milliseconds );
	};
}