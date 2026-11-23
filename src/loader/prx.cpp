#include <cellstatus.h>
#include <sys/prx.h>
#include <sys\ppu_thread.h>
#include <sys\Timer.h>
#include <cstddef>
#include <libpsutil.h>

SYS_MODULE_INFO( conex, 0, 1, 1);
SYS_MODULE_START( Entry );
SYS_MODULE_STOP( End );
SYS_MODULE_EXIT( End );
SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );	
SYS_LIB_EXPORT( Export, LIBNAME );

extern "C" int Export( void )
{
    return CELL_OK;
}

namespace Multiplayers
{
	bool IsIw5( void )
	{
		const char* part1 = ( const char* )0x73098E;
		const char* part2 = ( const char* )0x73099F;

		return ( strncmp( part1, "Modern Warfare", 14 ) == 0 && strncmp( part2, "3", 1 ) == 0 && libpsutil::filesystem::file_exists( "/dev_hdd0/tmp/conex/games/iw5mp.sprx" ) );
	}

	bool IsIw6( void )
	{
		const char* part1 = ( const char* )0xA8C512;
		const char* part2 = ( const char* )0xA8C521;

		return ( strncmp( part1, "Call of Duty", 12 ) == 0 && strncmp( part2, "Ghosts", 6 ) == 0 && libpsutil::filesystem::file_exists( "/dev_hdd0/tmp/conex/games/iw6mp.sprx" ) );
	}
}

extern "C" int Entry( void )
{
	sys_ppu_thread_t id;
	
	sys_ppu_thread_create( &id, []( uint64_t ) 
	{
		sys_timer_usleep( 20000000 );
	
		int modres = 0;
		void* argsp = NULL;

		if( Multiplayers::IsIw5() )
		{
			sys_prx_start_module( sys_prx_load_module( "/dev_hdd0/tmp/conex/games/iw5mp.sprx", 0, 0 ), 0, argsp, &modres, 0, NULL );
		}
		else if( Multiplayers::IsIw6() )
		{
			sys_prx_start_module( sys_prx_load_module( "/dev_hdd0/tmp/conex/games/iw6mp.sprx", 0, 0 ), 0, argsp, &modres, 0, NULL );
		}

		sys_ppu_thread_exit( 0 );
	}, NULL, 0x7FC, 0x100, 0, "Entry" );

    return SYS_PRX_RESIDENT;
}

extern "C" int End( void ) 
{
	return SYS_PRX_RESIDENT;
}