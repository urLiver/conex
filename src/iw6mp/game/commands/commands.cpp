#include "commands.hpp"

#include <cellstatus.h>
#include <sys/prx.h>
#include <sys\ppu_thread.h>
#include <sys\Timer.h>
#include <cstddef>
#include <libpsutil.h>
#include <cell/cell_fs.h>

#include "../../fs/write.hpp"

#include "../cmd_args/cmd_args.hpp"

#include "../dvar/dvars.hpp"

#include "../utils/utils.hpp"

#include "../../webman/webman.hpp"

namespace Game
{
    bool key_toggled_table[ 4 ][ 128 ] = { 0 }; 

    std::string key_cmd_table[ 4 ][ 128 ] = { 0 }; 
    
#pragma region HelperStructures
    #define G_Entity( index, offset ) ( 0xE04980 + ( index * 0x280  ) + offset )
    #define G_Client( index, offset ) ( *( int* )( 0xE04980 + ( index * 0x280 ) + 0x158 ) + offset )

    union Weapon
    {
        unsigned int data;
    };
#pragma endregion

#pragma region HelperFunctions
    inline int KeyToCmd( const char* key )
    {
        const char** commands= ( const char** )0xA85E38;
        int binding = 0;

        while( 1 )
        {
            const char* command = commands[ binding ];

            if( ! strcmp( command, key ) )
            {            
                break;
            }

            if( ++binding >= 0x2B )
            {
                LogWrite( "KeyToCmd: no binding found for key %s", key );
                break;
            }
        }

        return binding;
    }
        
    inline void SetEntityFlag( int local_client_num, int flag, const char* msg_on, const char* msg_off )
    {
        int old_flags;
        int client_num = CG_GetClientNum( local_client_num );

        old_flags = *( int* )G_Entity( client_num, 0x18C ) ^ flag;
        *( int* )G_Entity( client_num, 0x18C ) = old_flags;

        if( ( old_flags & flag ) != 0 )
        {
            CG_GameMessage( local_client_num, msg_on );
        }
        else
        {
            CG_GameMessage( local_client_num, msg_off );
        }
    }

    inline void SetClientFlag( int local_client_num, int flag, const char* msg_on, const char* msg_off )
    {
        int new_flags;
        int client_num = CG_GetClientNum( local_client_num );

        if( ( *( int* )G_Client( client_num, 0x331C ) & flag ) != 0 )
        {
            CG_GameMessage( local_client_num, msg_off );
            new_flags = *( int* )G_Client( client_num, 0x331C ) & ( 0xFFFFFFFF - flag );
        }
        else
        {
            CG_GameMessage( local_client_num, msg_on );
            new_flags = *( int* )G_Client( client_num, 0x331C ) | flag;
        }

        *( int* )G_Client( client_num, 0x331C ) = new_flags;
    }
    
    inline void G_GivePlayerWeapon( int local_client_num, const char* name )
    {
        static libpsutil::symbol<int( int ps, Weapon weapon, int dualWield, int startInAltMode, int usedBefore )> G_GivePlayerWeapon{ 0x2947FC };
        static libpsutil::symbol<Weapon*( Weapon* weapon, const char* name )>  G_GetWeaponForName{ 0x294D34 };
        static libpsutil::symbol<void( int ent, Weapon weapon, bool isAlternate, int count, int fillClip ) > Add_Ammo{ 0x24879C };

        Weapon weapon;
        weapon.data = 0;

        int client = CG_GetClientNum( local_client_num );

        G_GetWeaponForName( &weapon, name );

        if( ! G_GivePlayerWeapon( *( int* )G_Entity( client, 0x158 ), weapon, 0, 0, 0 ) )
        {
            LogWrite( "G_GivePlayerWeapon: called, but giving weapon failed for %s with index %i", name, weapon.data );
        }
        else
        {
            Add_Ammo( *( int* )G_Entity( client, 0x0 ), weapon, 0, 9999, 1 );
            
            std::string str = std::string( "Given: ^2" ).append( name );
            CG_GameMessage( local_client_num, str.c_str() );
        }
    }
#pragma endregion
    cmd_function_s Cmd_BindCmd_VAR;
    void Cmd_BindCmd( void )
    {
        if( CmdArgs_ArgC() == 3 )
        {
            int key = Key_StringToKeynum( CmdArgs_ArgV( 1 ) );

            if( key != -1 )
            {
                key = std::tolower( key );

                key_toggled_table[ CmdArgs_LocalClientNum() ][ key ] = 1;

                key_cmd_table[ CmdArgs_LocalClientNum() ][ key ] = std::string( CmdArgs_ArgV( 2 ) );
            }
            else
            {
                LogWrite( "invalid key for %s", CmdArgs_ArgV( 1 ) );
            }
        }
        else
        {
            LogWrite( "bind_cmd <key> <cmd> : binds a command to a key, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_UnbindCmd_VAR;
    void Cmd_UnbindCmd( void )
    {
        if( CmdArgs_ArgC() == 2 )
        {
            int key = Key_StringToKeynum( CmdArgs_ArgV( 1 ) );

            if( key != -1 )
            {
                key = std::tolower( key );

                key_toggled_table[ CmdArgs_LocalClientNum() ][ key ] = 0;
            }
            else
            {
                LogWrite( "invalid key for %s", CmdArgs_ArgV( 1 ) );
            }
        }
        else
        {
            LogWrite( "unbind_cmd <key> : unbinds a command from a key, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_Vstr_VAR;
    void Cmd_Vstr( void ) 
    {
        if ( CmdArgs_ArgC() == 2 )
        {
            const char* string = CmdArgs_ArgV( 1 );
            
            dvar_t* dvar = Dvar_FindVar( string );
            
            if ( dvar )
            {
                char dvar_type = dvar->type;
                if ( dvar_type == 6 || dvar_type == 7 )
                {
                    const char* dvar_newline = va( "%s\n", dvar->current.string );
            
                    Cbuf_InsertText( CmdArgs_LocalClientNum(), dvar_newline );
                }
                else
                {
                    LogWrite( "%s is not a string-based dvar, type: %i\n", dvar->name, dvar->type );
                }
            }
            else
            {
                LogWrite( "%s doesn't exist\n", string );
            }
        }
        else
        {
            LogWrite( "vstr <variablename> : execute a variable command, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_iPrintLn_VAR;
    void Cmd_iPrintLn( void )
    {
        if ( CmdArgs_ArgC() == 2 )
        {
            const char* message = CmdArgs_ArgV( 1 );

            CG_GameMessage( CmdArgs_LocalClientNum(), message );
        }
        else
        {
            LogWrite( "iprintln <message> : prints a message on the screen, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_iPrintLnBold_VAR;
    void Cmd_iPrintLnBold( void )
    {
        if ( CmdArgs_ArgC() == 2 )
        {
            const char* message = CmdArgs_ArgV( 1 );

            CG_GameMessageBold( CmdArgs_LocalClientNum(), message );
        }
        else
        {
            LogWrite( "iprintlnbold <message> : prints a message on the screen, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_Cmd_ExecFromDisk_VAR;
    void Cmd_ExecFromDisk( void ) 
    {
        if( CmdArgs_ArgC() == 2 )
        {
            const char* file_path = CmdArgs_ArgV( 1 );
            int file_descriptor;
            uint64_t pos;
            uint64_t nread;
            
            CellFsErrno error = cellFsOpen( file_path, 0, &file_descriptor, NULL, 0 );

            if( ! error )
            {
                CellFsStat stat;
                error = cellFsFstat( file_descriptor, &stat );

                if( ! error )
                {
                    void* space = _sys_malloc( ( size_t )( stat.st_size + 1 ) );
                    memset( space, 0 , ( size_t )( stat.st_size + 1 ) );
                    
                    cellFsLseek( file_descriptor, 0, CELL_FS_SEEK_SET, &pos );
                    error = cellFsRead( file_descriptor, space, stat.st_size, &nread );

                    if( ! error )
                    {
                        Cbuf_ExecuteBuffer( CmdArgs_LocalClientNum(), CmdArgs_ControllerIndex(), ( char* )space );
                    }
                    else
                    {
                        LogWrite( "%s cloudn't read file content\n", file_path );
                    }

                    _sys_free( space );
                    space = NULL;
                }
                else
                {
                    LogWrite( "%s cloudn't get file size\n", file_path );
                }
            }
            else
            {
                LogWrite( "%s doesn't exist\n", file_path );
            }

            cellFsClose( file_descriptor );
        }
        else
        {
            LogWrite( "execfromdisk <filepath> : loads a file into memory and runs it, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_DumpCommands_VAR;
    void Cmd_DumpCommands( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            cmd_function_s* command;
    
            for( command = ( cmd_function_s* )0x1063D28; command != NULL; command = command->next )
            {
                WriteFile( "/dev_hdd0/tmp/conex/data/iw6mp_commands.txt", "%s\n", command->name );
            }
        }
        else
        {
            LogWrite( "dump_commands called with arguments, while it shouldnt have any, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }
    
    cmd_function_s Cmd_DumpDvars_VAR;
    void Cmd_DumpDvars( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            sys_ppu_thread_t id;

            sys_ppu_thread_create( &id, []( uint64_t ) 
            {
                Dvar_DumpAllDvars();

                WebmanNotify( "Dumped dvars" );

                sys_ppu_thread_exit( 0 );
            }, NULL, 0x4AA, 0x7000, 0, "Cmd_DumpDvars" );
        }
        else
        {
            LogWrite( "dump_dvar called with arguments, while it shouldnt have any, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_DumpDvarsSimple_VAR;
    void Cmd_DumpDvarsSimple( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            sys_ppu_thread_t id;

            sys_ppu_thread_create( &id, []( uint64_t ) 
            {
                Dvar_DumpAllDvarsSimple();

                WebmanNotify( "Dumped dvars" );

                sys_ppu_thread_exit( 0 );
            }, NULL, 0x4AA, 0x7000, 0, "Cmd_DumpDvarsSimple" );
        }
        else
        {
            LogWrite( "dump_dvarsimple called with arguments, while it shouldnt have any, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_God_VAR;
    void Cmd_God( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {				
            SetEntityFlag( CmdArgs_LocalClientNum(), 1, "God: ^2On", "God: ^1Off" );
        }
        else
        {
            LogWrite( "god : sets flags, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_DemiGod_VAR;
    void Cmd_DemiGod( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            SetEntityFlag( CmdArgs_LocalClientNum(), 2, "Demi God: ^2On", "Demi God: ^1Off" );
        }
        else
        {
            LogWrite( "demigod : sets flags, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_NoTarget_VAR;
    void Cmd_NoTarget( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            SetEntityFlag( CmdArgs_LocalClientNum(), 4, "No Target: ^2On", "No Target: ^1Off" );
        }
        else
        {
            LogWrite( "notarget : sets flags, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_NoClip_VAR;
    void Cmd_NoClip( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            SetClientFlag( CmdArgs_LocalClientNum(), 1, "No Clip: ^2On", "No Clip: ^1Off" );
        }
        else
        {
            LogWrite( "noclip : sets flags, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_Ufo_VAR;
    void Cmd_Ufo( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            SetClientFlag( CmdArgs_LocalClientNum(), 2, "Ufo: ^2On", "Ufo: ^1Off" );
        }
        else
        {
            LogWrite( "ufo : sets flags, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s Cmd_Give_VAR;
    void Cmd_Give( void )
    {
        if( CmdArgs_ArgC() == 2 )
        {
            G_GivePlayerWeapon( CmdArgs_LocalClientNum(), CmdArgs_ArgV( 1 ) );
        }
        else
        {
            LogWrite( "give <weaponname> : gives you the weapons with that name, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

    cmd_function_s cmd_functions[ 45 ];
    void ExecBinding( void )
    {
        static libpsutil::symbol<void( int localClientNum, int button, int cmd, int idk )> ExecBinding{ 0x5E600C };

        if( CmdArgs_ArgC() == 1 )
        {
            ExecBinding( CmdArgs_LocalClientNum(), KeyToCmd( CmdArgs_ArgV( 0 ) ), KeyToCmd( CmdArgs_ArgV( 0 ) ), 0 );
        }
        else
        {
            LogWrite( "%s : called with arguments while it shouldn't have any, argc: %i", CmdArgs_ArgV( 0 ), CmdArgs_ArgC() );
        }
    }

    void SetupCommands()
    {
        const char** commands = ( const char** )0xA85E38;
        int binding = 0;

        while( 1 )
        {
            const char* command = commands[ binding ];

            Cmd_AddCommandInternal( command, ExecBinding, &cmd_functions[ binding ] );

            if( ++binding >= 0x2B )
            {
                break;
            }
        }
		
        Cmd_AddCommandInternal( "bind_cmd", Cmd_BindCmd, &Cmd_BindCmd_VAR );
        Cmd_AddCommandInternal( "unbind_cmd", Cmd_UnbindCmd, &Cmd_UnbindCmd_VAR );
        Cmd_AddCommandInternal( "god", Cmd_God, &Cmd_God_VAR );
        Cmd_AddCommandInternal( "demigod", Cmd_DemiGod, &Cmd_DemiGod_VAR );
        Cmd_AddCommandInternal( "notarget", Cmd_NoTarget, &Cmd_NoTarget_VAR );
        Cmd_AddCommandInternal( "ufo", Cmd_Ufo, &Cmd_Ufo_VAR );
        Cmd_AddCommandInternal( "noclip", Cmd_NoClip, &Cmd_NoClip_VAR );
        Cmd_AddCommandInternal( "give", Cmd_Give, &Cmd_Give_VAR );
        Cmd_AddCommandInternal( "dump_commands", Cmd_DumpCommands, &Cmd_DumpCommands_VAR );
        Cmd_AddCommandInternal( "dump_dvars", Cmd_DumpDvars, &Cmd_DumpDvars_VAR );
        Cmd_AddCommandInternal( "dump_dvarsimple", Cmd_DumpDvarsSimple, &Cmd_DumpDvarsSimple_VAR );
        Cmd_AddCommandInternal( "vstr", Cmd_Vstr, &Cmd_Vstr_VAR );
        Cmd_AddCommandInternal( "iprintln", Cmd_iPrintLn, &Cmd_iPrintLn_VAR );
        Cmd_AddCommandInternal( "iprintlnbold", Cmd_iPrintLnBold, &Cmd_iPrintLnBold_VAR );
        Cmd_AddCommandInternal( "execfromdisk", Cmd_ExecFromDisk, &Cmd_Cmd_ExecFromDisk_VAR );
    }
}