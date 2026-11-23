#include <cellstatus.h>
#include <sys/prx.h>
#include <sys\ppu_thread.h>
#include <sys\Timer.h>
#include <cstddef>

#include "webman/webman.hpp"

#include "game/main.hpp"

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

extern "C" int Entry( void )
{
	sys_ppu_thread_t id;
	
	sys_ppu_thread_create( &id, []( uint64_t ) 
	{
		Game::Main();

		WebmanNotify( "Loading: Modern Warfare 3 ( MP )" );

		sys_ppu_thread_exit( 0 );
	}, NULL, 0x7FC, 0x100, 0, "Entry" );

    return SYS_PRX_RESIDENT;
}

extern "C" int End( void ) 
{
	sys_ppu_thread_t id;
	
	sys_ppu_thread_create( &id, []( uint64_t ) 
	{
		WebmanNotify( "Stopping" );
	
		Game::End();

		sys_ppu_thread_exit( 0 );
	}, NULL, 0x7FC, 0x100, 0, "End" );

	return SYS_PRX_RESIDENT;
}