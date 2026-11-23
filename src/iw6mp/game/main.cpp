#include "main.hpp"

#include <string>
#include <libpsutil.h>
#include <cell\pad.h>

#include "utils/utils.hpp"

#include "dvar/dvars.hpp"

#include "commands/commands.hpp"

#include "../webman/webman.hpp"
#include "../fs/write.hpp"

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
	libpsutil::memory::detour* detour_ClientConnectedToServer;
	void hook_ClientConnectedToServer( int local_client_num, const char* function_name )
	{
		return;
	}

	libpsutil::memory::detour* detour_Entitlements_IsIDUnlocked;
	bool hook_Entitlements_IsIDUnlocked( int controllerindex, int a2 )
	{
		return Dvar_GetBool( "unlock_dlc_content" ) ? true : detour_Entitlements_IsIDUnlocked->invoke<bool>( controllerindex, a2 );
	}

	libpsutil::memory::detour* detour_Content_DoWeHaveDLCPackByName;
	bool hook_Content_DoWeHaveDLCPackByName( int controllerindex, unsigned int a2 )
	{
		return Dvar_GetBool( "unlock_dlc_content" ) ? true : detour_Content_DoWeHaveDLCPackByName->invoke<bool>( controllerindex, a2 );
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

	libpsutil::memory::detour* detour_MenuPaint;
	void hook_MenuPaint( int localClientNum )
	{
		detour_MenuPaint->invoke( localClientNum );
	
		if( *(char*)0x2357DF0 != 0x01 )
		{
			HelperConsole_Do();
		}
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
		detour_ClientConnectedToServer->~detour();
		detour_CG_TranslateGamepadButton->~detour();
		detour_Entitlements_IsIDUnlocked->~detour();
		detour_Content_DoWeHaveDLCPackByName->~detour();
		detour_StringTable_GetAsset->~detour();
		detour_LiveStorage_IsItemUnlockedFromTable_LocalClient->~detour();
		detour_MenuPaint->~detour();
		detour_sys_io_cellPadGetData->~detour();
	}
	
	void Main( void )
	{
		SetupCommands();
		
		Dvar_RegisterBool( "unlock_dlc_content", 1, 0 );
		Dvar_RegisterBool( "unlock_any_attachment_combo", 1, 0 );
		
		Dvar_SetIntByName( "extinction_map_selection_enabled", 1 );
		Dvar_SetIntByName( "extendedLoadoutsEnable", 1 );
		Dvar_SetIntByName( "igs_announcer", 3 );
		Dvar_SetIntByName( "igs_swp", 1 );
		Dvar_SetIntByName( "igs_shp", 1 );
		Dvar_SetIntByName( "igs_svp", 1 );
		Dvar_SetIntByName( "igs_sve", 1 );
		Dvar_SetIntByName( "igs_svs", 1 );
		Dvar_SetIntByName( "igs_svr", 1 );
		Dvar_SetIntByName( "igs_swap", 1 );
		Dvar_SetIntByName( "igs_fo", 1 );
		Dvar_SetIntByName( "igs_td", 1 );
		Dvar_SetIntByName( "igs_sripper", 1 );
		Dvar_SetIntByName( "igs_smappacks", 1 );
		Dvar_SetIntByName( "igs_sosp", 1 );
		Dvar_SetIntByName( "igs_s1", 1 );
		Dvar_SetIntByName( "igs_crossgame", 1 );
		
		detour_ClientConnectedToServer = new libpsutil::memory::detour( static_cast<int>( 0x6BDE8C ), hook_ClientConnectedToServer );
		detour_CG_TranslateGamepadButton = new libpsutil::memory::detour( static_cast<int>( 0x15925C ), hook_CG_TranslateGamepadButton );
		detour_Entitlements_IsIDUnlocked = new libpsutil::memory::detour( static_cast<int>( 0x42EA98 ), hook_Entitlements_IsIDUnlocked );
		detour_Content_DoWeHaveDLCPackByName = new libpsutil::memory::detour( static_cast<int>( 0x5EF42C ), hook_Content_DoWeHaveDLCPackByName );
		detour_StringTable_GetAsset = new libpsutil::memory::detour( static_cast<int>( 0x382888 ), hook_StringTable_GetAsset );
		detour_LiveStorage_IsItemUnlockedFromTable_LocalClient = new libpsutil::memory::detour( static_cast<int>( 0x54B634 ), hook_LiveStorage_IsItemUnlockedFromTable_LocalClient );
		
		// Hooks for the Helper Console
		detour_MenuPaint = new libpsutil::memory::detour( static_cast<int>( 0x36DBC4 ), hook_MenuPaint );
		detour_sys_io_cellPadGetData = new libpsutil::memory::detour( static_cast<int>( 0x81951C ), hook_sys_io_cellPadGetData );

		if( libpsutil::filesystem::file_exists( "/dev_hdd0/tmp/conex/data/iw6_onstart.cfg" ) )
		{
			Cbuf_AddText( 0, ";execfromdisk \"/dev_hdd0/tmp/conex/data/iw6_onstart.cfg\";" );
		}
	}
}