#include "helper_console.hpp"

#include "../dvar/dvars.hpp"

#include "../cmd_args/cmd_args.hpp"

#include <libpsutil.h>
#include <ctype.h>

#define FONT_SIZE 0.2f
#define CONSOLE_PADDING_LEFT 6.0f
#define CONSOLE_PADDING_TOP 4.0f
#define TEXT_PADDING 2.0f
#define MAX_MATCHES_SHOWN 31

#define PAD_LEFT ( 1 << 7 )
#define PAD_RIGHT ( 1 << 5 )
#define PAD_L3 ( 1 << 1 )

namespace Game
{
	static libpsutil::symbol< int ( void* font, float scale ) > UI_TextHeight{ 0x348198 };
    static libpsutil::symbol< void ( void* scrPlace, double x, double y, double w, double h, int horzAlign, int vertAlign, const float* color, void* material ) > UI_DrawHandlePic{ 0x332078 };
	static libpsutil::symbol< void ( void* scrPlace, const char* text, int maxChars, void* font, double x, double y, int horzAlign, int vertAlign, double scale, const float* color, int style ) > UI_DrawText{ 0x348840 };
    static libpsutil::symbol<void*( const char *name, int imageTrack )> Material_RegisterHandle{ 0x46540C };
	static libpsutil::symbol<void*( const char* name, int track )> R_RegisterFont{ 0x5608A0 };
	static libpsutil::symbol<void*( void )> ScrPlace_GetActivePlacement{ 0x1B8CCC };
	static libpsutil::symbol<void( int localClientNum, int mask )> Key_AddCatcher{ 0x190128 };
	static libpsutil::symbol<void( int localClientNum, int mask )> Key_RemoveCatcher{ 0x190154 };
	static libpsutil::symbol<bool( int localClientNum, int mask )> Key_IsCatcherActive{ 0x1900F4 };
	static libpsutil::symbol<int( void )> Sys_Milliseconds{ 0x3EB584 };

    struct Timer
	{
		int time;
		int delay;

		bool Ready( void ) 
        {
		    return ( ( Sys_Milliseconds() - time ) >= delay );
	    }

		void Wait( int milliseconds )
        {
		    time = Sys_Milliseconds();
		    delay = milliseconds;
	    }
	};

	struct ConsoleVariables
	{
        void* font;
        void* material;
        void* screen_placement;

        int button = 0;

        float left = 0;
        float top = 0;
        float right = 0;
        float bottom = 0;
        float fontHeight = 0;
        float textX = 0;
        float textY = 0;

        int matchCount = 0;
        int page = 0;
        int lineIndex = 1;
    
        float inputBoxColor[ 4 ] = { 0.25f, 0.25f, 0.2f, 1.0f }; // This does not work on PS3???
        float inputHintBoxColor[ 4 ] = { 0.40000001f, 0.40000001f, 0.34999999f, 1.0f };
        float textColor[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float commandTextColor[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
        
        Timer timer = Timer();
	};

	ConsoleVariables console_variales = ConsoleVariables();
    
    #pragma region Rendering_Helpers
    void ConDraw_Box( float textX, float textY, float w, float h, const float *color)
    {
        float darkColor[ 4 ] = { color[ 0 ] * 0.5f, color[ 1 ] * 0.5f, color[ 2 ] * 0.5f, color[ 3 ] };
        
        UI_DrawHandlePic( console_variales.screen_placement, textX, textY, w, h, 4, 4, color, console_variales.material );
        UI_DrawHandlePic( console_variales.screen_placement, textX, textY, 1.0f, h, 4, 4, darkColor, console_variales.material );
        UI_DrawHandlePic( console_variales.screen_placement, textX + w - 1.0f, textY, 1.0f, h, 4, 4, darkColor, console_variales.material );
        UI_DrawHandlePic( console_variales.screen_placement, textX, textY, w, 1.0f, 4, 4, darkColor, console_variales.material );
        UI_DrawHandlePic( console_variales.screen_placement, textX, textY + h - 1.0f, w, 1.0f, 4, 4, darkColor, console_variales.material );
    }

    void ConsoleDrawBox( int lines, const float* color )
    {
        ConDraw_Box( console_variales.left, console_variales.top, console_variales.right - console_variales.left, ( float )lines * console_variales.fontHeight + 4.0f, color );
    }
    
    void ConsoleDrawText( const char *str, const float *color )
    {
        UI_DrawText( console_variales.screen_placement, str, 0xFFFFFFFF, console_variales.font, console_variales.textX, console_variales.textY + console_variales.fontHeight, 4, 4, FONT_SIZE, color, 0 );
    }

    void ConsolePreviewDvar( dvar_t* dvar, void* data )
    {
        if( console_variales.lineIndex > console_variales.page * MAX_MATCHES_SHOWN && console_variales.lineIndex <= ( console_variales.page + 1 ) * MAX_MATCHES_SHOWN )
        {
            ConsoleDrawText( dvar->name, console_variales.textColor );

            console_variales.textX += 230.f;

            ConsoleDrawText( Dvar_ValueToString( dvar ), console_variales.textColor );
            
            console_variales.textX -= 230.f;

            console_variales.textY += console_variales.fontHeight;
        }

        console_variales.lineIndex++;
    }
    
    template<class T>
    const T& Clamp(const T& v, const T& lo, const T& hi)
    {
        return v < lo ? lo : hi < v ? hi : v;
    }

    #pragma endregion

	void HelperConsole_Do()
	{
        if( console_variales.timer.Ready() )
        {
            if( console_variales.button & PAD_L3 ) 
            {
                if( ! Key_IsCatcherActive( 0, 1 ) )
                {
                    Key_AddCatcher( 0, 1 );
                }
                else
                {
                    Key_RemoveCatcher( 0, ~1 );
                }

                console_variales.timer.Wait( 250 );
            }
            else if( console_variales.button & PAD_LEFT && Key_IsCatcherActive( 0, 1 ) )
            {
                if( console_variales.page > 0 )
                {
                    console_variales.page--;

                    console_variales.timer.Wait( 250 );
                }
            }
            else if( console_variales.button & PAD_RIGHT && Key_IsCatcherActive( 0, 1 ) )
            {
                if( Clamp( console_variales.matchCount - ( console_variales.page + 1 ) * MAX_MATCHES_SHOWN, 0, MAX_MATCHES_SHOWN ) != 0 )
                {
                   console_variales.page++;

                    console_variales.timer.Wait( 250 );
                }
            }
        }

        if( ! Key_IsCatcherActive( 0, 1 ) )
        {
            return;
        }
        
        static bool init = true;
        if( init )
        {
            init = false;
                    
		    console_variales.font = R_RegisterFont( "fonts/consoleFont", 0 );
            console_variales.material = Material_RegisterHandle( "white", 0 );
            console_variales.screen_placement = ScrPlace_GetActivePlacement();

            console_variales.left = CONSOLE_PADDING_LEFT;
            console_variales.top = CONSOLE_PADDING_TOP;
            console_variales.right = 640.f - ( CONSOLE_PADDING_LEFT * 2 );
            console_variales.bottom = 480.0f - ( CONSOLE_PADDING_TOP * 2 );
            console_variales.fontHeight = ( float )UI_TextHeight( console_variales.font, FONT_SIZE );
        
            console_variales.inputBoxColor[ 0 ] = 0.25;
            console_variales.inputBoxColor[ 1 ] = 0.25;
            console_variales.inputBoxColor[ 2 ] = 0.2f;
            console_variales.inputBoxColor[ 3 ] = 1.0f;

            console_variales.inputHintBoxColor[ 0 ] = 0.40000001f;
            console_variales.inputHintBoxColor[ 1 ] = 0.40000001f;
            console_variales.inputHintBoxColor[ 2 ] = 0.34999999f;
            console_variales.inputHintBoxColor[ 3 ] = 1.0f;
        
            console_variales.textColor[ 0 ] = 1.0f;
            console_variales.textColor[ 1 ] = 1.0f;
            console_variales.textColor[ 2 ] = 1.0f;
            console_variales.textColor[ 3 ] = 1.0f;

            console_variales.commandTextColor[ 0 ] = 0.75f;
            console_variales.commandTextColor[ 1 ] = 1.0f;
            console_variales.commandTextColor[ 2 ] = 0.5f;
            console_variales.commandTextColor[ 3 ] = 1.0f;
        }
        
        console_variales.textX = console_variales.left + TEXT_PADDING;
        console_variales.textY = console_variales.top + TEXT_PADDING;
        console_variales.matchCount = 0;
        
        cmd_function_s* command;
    
        for( command = ( cmd_function_s* )0x1063D28; command != NULL; command = command->next )
        {
            if( command != NULL && command->name != NULL && strcmp( command->name, "" ) )
            {
                console_variales.matchCount++;
            }
        }

        Dvar_ForEach( []( dvar_t* dvar, void* data ) {
                console_variales.matchCount++;
        }, NULL );
        
        char buffer[ 128 ];

        sprintf( buffer, "%i matches, %i page showing %i - %i, navigate with left and right arrow keys", console_variales.matchCount, console_variales.page, console_variales.page * MAX_MATCHES_SHOWN, ( console_variales.page + 1 ) * MAX_MATCHES_SHOWN - 1 );

        ConsoleDrawBox( 1 + Clamp( console_variales.matchCount - console_variales.page * MAX_MATCHES_SHOWN, 0, MAX_MATCHES_SHOWN ), console_variales.inputHintBoxColor );
        ConsoleDrawText( buffer, console_variales.textColor );
        
        console_variales.textY += console_variales.fontHeight;
        console_variales.lineIndex = 1;

        for( command = ( cmd_function_s* )0x1063D28; command != NULL; command = command->next )
        {
            if( command != NULL && command->name != NULL && strcmp( command->name, "" ) )
            {
                if( console_variales.lineIndex > console_variales.page * MAX_MATCHES_SHOWN && console_variales.lineIndex <= ( console_variales.page + 1 ) * MAX_MATCHES_SHOWN )
                {
                    ConsoleDrawText( command->name, console_variales.commandTextColor );

                    console_variales.textY += console_variales.fontHeight;
                }

                console_variales.lineIndex++;
            }
        }

        Dvar_ForEach( ConsolePreviewDvar, NULL );
	}

	void HelperConsole_UpdateButtons( int button )
	{
		console_variales.button = button;
	}
}