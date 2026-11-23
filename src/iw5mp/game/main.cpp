#include "main.hpp"

#include <string>
#include <libpsutil.h>
#include <cell\pad.h>

#include "utils/utils.hpp"

#include "dvar/dvars.hpp"

#include "commands/commands.hpp"

#include "../webman/webman.hpp"

#include "helper_console/helper_console.hpp"

namespace Game
{
	#pragma region Structs
	struct StringTableCell
	{
		const char *string;
		int hash;
	};

	struct StringTable
	{
		const char *name;
		int columnCount;
		int rowCount;
		StringTableCell *values;
	};
	#pragma endregion

	#pragma region Hooks
	libpsutil::memory::detour* detour_CG_TranslateGamepadButton;
	bool hook_CG_TranslateGamepadButton( int localClientNum, int button, int cmd )
	{
		if( key_toggled_table[ localClientNum ][ button ] )
		{
			const char* str = key_cmd_table[ localClientNum ][ button ].c_str();

			Cbuf_AddText( localClientNum, str );
			Cbuf_AddText( localClientNum, "\n" );

			return true;
		}

		return detour_CG_TranslateGamepadButton->invoke<bool>( localClientNum, button, cmd );
	}

	libpsutil::memory::detour* detour_LiveStorage_IsItemUnlockedFromTable_LocalClient;
	bool hook_LiveStorage_IsItemUnlockedFromTable_LocalClient( int localClientNum, const StringTable *unlockTable, int row )
	{
		return Dvar_GetBool( "unlock_dlc_content" ) ? true : detour_LiveStorage_IsItemUnlockedFromTable_LocalClient->invoke<bool>( localClientNum, unlockTable, row );
	}

	libpsutil::memory::detour* detour_StringTable_GetAsset;
	void hook_StringTable_GetAsset( const char *filename, StringTable **tablePtr )
	{
		detour_StringTable_GetAsset->invoke( filename, tablePtr );
	
		StringTable* table = *tablePtr;

		if( Dvar_GetBool( "unlock_any_attachment_combo" ) && ! strcmp( "mp/attachmentCombos.csv", filename ) )
		{
			for( int row = 0; row < table->rowCount; row++ )
			{
				for ( int col = 0; col < table->columnCount; col++ )
				{
					if( ! strcmp( table->values[ row * table->columnCount + col ].string, "no" ) )
					{
						table->values[ row * table->columnCount + col ].string = "";
						table->values[ row * table->columnCount + col ].hash = 0;
					}
				}
			}
		}
	}

	libpsutil::memory::detour* detour_Com_Error;
	void hook_Com_Error( int code, const char *fmt, ... )
	{
		if( ! strcmp( fmt, "Trying to set persistent player data while connected to a server (%s, clcState = %i)!" ) )
		{
			return;
		}

	    va_list args;
		va_start( args, fmt );
		
		detour_Com_Error->invoke( code, fmt, args );

		va_end( args );
	}

	libpsutil::memory::detour* detour_UI_DrawBuildNumber;
	void hook_UI_DrawBuildNumber( int localClientNum )
	{
		detour_UI_DrawBuildNumber->invoke( localClientNum );
	
		HelperConsole_Do();
	}

	libpsutil::memory::detour* detour_sys_io_cellPadGetData;
	int hook_sys_io_cellPadGetData( unsigned int port, CellPadData* data ) 
	{
		if ( port != 0 )
		{	
			return detour_sys_io_cellPadGetData->invoke<int>( port, data );
		}

		CellPadData PadData = CellPadData();

		int ret = cellPadGetData( port, &PadData );
		std::memcpy( data, &PadData, sizeof( CellPadData ) );
		
		HelperConsole_UpdateButtons( ( ( PadData.button[ 3 ] << 8 ) + PadData.button[ 2 ] )  );

		return ret;
	}

	#pragma endregion
	
	void End( void )
	{
		detour_Com_Error->~detour();
		detour_CG_TranslateGamepadButton->~detour();
		detour_LiveStorage_IsItemUnlockedFromTable_LocalClient->~detour();
		detour_StringTable_GetAsset->~detour();
		detour_UI_DrawBuildNumber->~detour();
		detour_sys_io_cellPadGetData->~detour();
	}
	
	void Main( void )
	{
		SetupCommands();
		
		Dvar_RegisterBool( "unlock_dlc_content", 1, 0 );
		Dvar_RegisterBool( "unlock_any_attachment_combo", 1, 0 );
		
		detour_Com_Error = new libpsutil::memory::detour( static_cast<int>( 0x1F6A08 ), hook_Com_Error );
		detour_CG_TranslateGamepadButton = new libpsutil::memory::detour( static_cast<int>( 0x72664 ), hook_CG_TranslateGamepadButton );
		detour_LiveStorage_IsItemUnlockedFromTable_LocalClient = new libpsutil::memory::detour( static_cast<int>( 0x1EB92C ), hook_LiveStorage_IsItemUnlockedFromTable_LocalClient );
		detour_StringTable_GetAsset = new libpsutil::memory::detour( static_cast<int>( 0x28BE84 ), hook_StringTable_GetAsset );
		
		// Hooks for the Helper Console
		detour_UI_DrawBuildNumber = new libpsutil::memory::detour( static_cast<int>( 0x252A90 ), hook_UI_DrawBuildNumber );
		detour_sys_io_cellPadGetData = new libpsutil::memory::detour( static_cast<int>( 0x51B6F4 ), hook_sys_io_cellPadGetData );

		if( libpsutil::filesystem::file_exists( "/dev_hdd0/tmp/conex/data/iw5mp_onstart.cfg" ) )
		{
			Cbuf_AddText( 0, ";execfromdisk \"/dev_hdd0/tmp/conex/data/iw5mp_onstart.cfg\";" );
		}
	}
}