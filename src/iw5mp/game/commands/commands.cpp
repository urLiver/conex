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
    #define G_Entity( index, offset ) ( 0xFCA280 + ( index * 0x280  ) + offset )
    #define G_Client( index, offset ) ( *( int* )( 0xFCA280 + ( index * 0x280 ) + 0x158 ) + offset )

    union Weapon
    {
        unsigned int data;
    };

    struct Xuid
    {
        uint32_t low;
        uint32_t high;
        
        Xuid( uint32_t low = 0, uint32_t high = 0 ): low( low ), high( high ) { } 
    };

    Xuid xuids[ 53 ] = 
    {
        { 0x3BF9D611, 0xC0CC50CB }, // 
        { 0x3C65DF15, 0x95262A83 }, // EraEchoz
        { 0xF7C191B5, 0x8AD98549 }, // zAxno-
        { 0x880B2686, 0x33EB1562 }, // Barcode
        { 0x5A4DB151, 0x154C7F9C }, // Randy
        { 0xD47DE692, 0x96FC307B }, // SlipknotNYC
        { 0x2FEF394B, 0x491D6947 }, // John_Leki
        { 0x444B46F2, 0x6AB82054 }, // IMC Pigz
        { 0x05F3A47B, 0x10AA0072 }, // XUID Yoda
        { 0xF0C05D3A, 0x84FCF720 }, // Mazenmalik
        { 0xAFBCCA32, 0x8A38D43B }, // USA im the coolest
        { 0xF38AB745, 0xFF51B57A }, // Usances
        { 0x77DEEC0E, 0x2C6A6D86 }, // Whipdog
        { 0x91F36567, 0xC813E12F }, // Akimbo banton
        { 0x3CF4476A, 0x070DBE87 }, // Black Knight
        { 0xA5E49B0B, 0x1BC3343D }, // Black Knight
        { 0x9B4CB27B, 0xEA13D254 }, // Black Knight
        { 0xE8A44809, 0x398285EA }, // Black Knight
        { 0x57830C1C, 0x9AB6F907 }, // Black Knight
        { 0xF0463CB5, 0xC841FA7E }, // Black Knight
        { 0x1209BA22, 0x60DF789D }, // Black Knight
        { 0x294B4F83, 0xE1B704A0 }, // decentplayer94
        { 0xA0A0A101, 0x01A41CFE }, // luisantonio1204
        { 0x1BF9336E, 0xFAA22BCD }, // luisantonio1204
        { 0x1209BA22, 0x60DF789D }, // TANQUE1999NL
        { 0x5A4DB151, 0x154C7F9C }, // TANQUE1999NL
        { 0x971F7B0B, 0xF92C1B8B }, // TANQUE1999NL
        { 0xFCE788AE, 0xE5F49F3B }, // Nightwar_2012
        { 0xF38AB745, 0xFF51B57A }, // Sky_Vodkaa
        { 0xC2BF55B7, 0xD0CB0FE9 }, // travlack 36
        { 0x2D9AB0EF, 0xDF7E0B17 }, // Dbag
        { 0x4188B8A3, 0x7BFF2B39 }, // Mommhy-
        { 0xDC2916E1, 0x8261B17D }, // colby
        { 0xF9E68F1A, 0x06C44624 }, // MRGRINGAS89
        { 0x5A66E174, 0x82E3B4BE }, // tbird340
        { 0x434829A9, 0x790791D3 }, // james_sboy
        { 0x86A4ED98, 0xE9471EB3 }, // TOPSHOTTA_AKMODZ
        { 0xFBC83D4C, 0x426B5C63 }, // frandiber_RD807
        { 0x3CF4476A, 0x070DBE87 }, // M4
        { 0x57830C1C, 0x9AB6F907 }, // M4
        { 0xE14E8080, 0xB6A56020 }, // Pfyzy
        { 0xC7E715CA, 0x56799F76 }, // tpity
        { 0x495A6654, 0xF9EF664E }, // Zexful
        { 0x495A6654, 0xF9EF664E }, // Juniors imc
        { 0x5F00C4A4, 0x6015D052 }, // Juniors imc
        { 0x4751BFF7, 0xCCA81BE5 }, // Dollarsigns
        { 0xD3F285FA, 0x3D09DAF2 }, // Faze_Chick
        { 0xBE4E063E, 0x39611236 }, // IGhostOG
        { 0x8727CE52, 0xFFFA8833 }, // Tipidi23
        { 0x7E4F5DBF, 0x72F4E868 }, // Sarbeh_g
        { 0x9F5039C3, 0xEBCC560C }, // UrM0m98880_2
        { 0x57830C1C, 0x9AB6F907 }, // wikmy
        { 0x18B630FF, 0x2EC10595 } // proXxchrisxX
    };
#pragma endregion

#pragma region HelperFunctions
    inline int KeyToCmd( const char* key )
    {
        const char** commands= ( const char** )0x728ADC;
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

        old_flags = *( int* )G_Entity( client_num, 0x184 ) ^ flag;
        *( int* )G_Entity( client_num, 0x184 ) = old_flags;

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

        if( ( *( int* )G_Client( client_num, 0x35FC ) & flag ) != 0 )
        {
            CG_GameMessage( local_client_num, msg_off );
            new_flags = *( int* )G_Client( client_num, 0x35FC ) & ( 0xFFFFFFFF - flag );
        }
        else
        {
            CG_GameMessage( local_client_num, msg_on );
            new_flags = *( int* )G_Client( client_num, 0x35FC ) | flag;
        }

        *( int* )G_Client( client_num, 0x35FC ) = new_flags;
    }
    
    inline void G_GivePlayerWeapon( int local_client_num, const char* name )
    {
        static libpsutil::symbol<int( int ps, Weapon weapon, int alt )> G_GivePlayerWeapon{ 0x1C3034 };
        static libpsutil::symbol<Weapon*( Weapon* weapon, const char* name )>  G_GetWeaponForName{ 0x1C34B8 };
        static libpsutil::symbol<void( int ent, Weapon weapon, bool isAlternate, int count, int fillClip ) > Add_Ammo{ 0x18A29C };

        Weapon weapon;
        weapon.data = 0;

        int client = CG_GetClientNum( local_client_num );

        G_GetWeaponForName( &weapon, name );

        if( ! G_GivePlayerWeapon( *( int* )G_Entity( client, 0x158 ), weapon, 0 ) )
        {
            LogWrite( "G_GivePlayerWeapon: called, but giving weapon failed for %s with index %i", name, weapon.data );
        }
        else
        {
            Add_Ammo( G_Entity( client, 0x0 ), weapon, 0, 9999, 1 );
            
            std::string str = std::string( "Given: ^2" ).append( name );
            CG_GameMessage( local_client_num, str.c_str() );
        }
    }
#pragma endregion

    cmd_function_s Cmd_Xuid_VAR;
    void Cmd_Xuid( void )
    {
        if( CmdArgs_ArgC() == 1 )
        {
            sys_ppu_thread_t id;

            sys_ppu_thread_create( &id, []( uint64_t ) 
            {
                const int count = sizeof( xuids ) / sizeof( xuids[ 0 ] );
                Xuid random = xuids[ ( int )( libpsutil::math::rand() % count ) ];

                uint32_t low  = random.low;
                uint32_t high = random.high;
                
                *( uint32_t* )0x0073E4C0 = low;
                *( uint32_t* )( 0x0073E4C0 + 4 ) = high;

                for( int i = 0; i < 18; i++ )
                {
                    uintptr_t addr = 0x008A3C20 + ( i * 376 );
                    *( uint32_t* )addr = low;
                    *( uint32_t* )( addr + 4 ) = high;
                }
                
                *( uint32_t* )0x01BAF690 = low;
                *( uint32_t* )( 0x01BAF690 + 4 ) = high;

                *( uint32_t* )0x01C4E028 = low;
                *( uint32_t* )( 0x01C4E028 + 4 ) = high;

                *( uint32_t* )0x01C4E5E0 = low;
                *( uint32_t* )( 0x01C4E5E0 + 4 ) = high;

                *( uint32_t* )0x01C4E7E8 = low;
                *( uint32_t* )( 0x01C4E7E8 + 4 ) = high;

                *( uint32_t* )0x01CF4CF0 = low;
                *( uint32_t* )( 0x01CF4CF0 + 4 ) = high;

                *( uint32_t* )0x01C63938 = low;
                *( uint32_t* )( 0x01C63938 + 4 ) = high;
                
                *( uint32_t* )0x01D5B1C0 = low;
                *( uint32_t* )( 0x01D5B1C0 + 4 ) = high;
            
                *( uint32_t* )0x01D63B48 = low;
                *( uint32_t* )( 0x01D63B48 + 4 ) = high;
                
                *( uint32_t* )0x01D8B5E8 = low;
                *( uint32_t* )( 0x01D8B5E8 + 4 ) = high;
                
                *( char* )0x01BAF65B = 1;

                WebmanNotify( "Spoofed XUID" );

                sys_ppu_thread_exit( 0 );
            }, NULL, 0x4AA, 0x7000, 0, "XUID" );
        }
        else
        {
            LogWrite( "xuid : spoof your xuid to a random one of a list, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
        }
    }

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
    
            for( command = ( cmd_function_s* )0x11B5A30; command != NULL; command = command->next )
            {
                WriteFile( "/dev_hdd0/tmp/conex/data/iw5mp_commands.txt", "%s\n", command->name );
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
            LogWrite( "dump_dvarssimple called with arguments, while it shouldnt have any, CmdArgs_ArgC: %i\n", CmdArgs_ArgC() );
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
        static libpsutil::symbol<void( int localClientNum, int button, int cmd, int idk )> ExecBinding{ 0xD2904 };

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
        const char** commands = ( const char** )0x728ADC;
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
		
        Cmd_AddCommandInternal( "xuid", Cmd_Xuid, &Cmd_Xuid_VAR );
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
        Cmd_AddCommandInternal( "dump_dvarssimple", Cmd_DumpDvarsSimple, &Cmd_DumpDvarsSimple_VAR );
        Cmd_AddCommandInternal( "vstr", Cmd_Vstr, &Cmd_Vstr_VAR );
        Cmd_AddCommandInternal( "iprintln", Cmd_iPrintLn, &Cmd_iPrintLn_VAR );
        Cmd_AddCommandInternal( "iprintlnbold", Cmd_iPrintLnBold, &Cmd_iPrintLnBold_VAR );
        Cmd_AddCommandInternal( "execfromdisk", Cmd_ExecFromDisk, &Cmd_Cmd_ExecFromDisk_VAR );
    }
}