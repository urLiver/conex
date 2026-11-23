#include "cmd_args.hpp"

#include <stdlib.h>
#include <libpsutil.h>

namespace Game
{
	CmdArgs* CmdArgs_GetCmdArgs( void )
	{
		return reinterpret_cast<CmdArgs*>( 0x103EBB0 );
	}

	int CmdArgs_ArgC( void ) 
	{
		CmdArgs* cmd_args = CmdArgs_GetCmdArgs();

		if( cmd_args == NULL )
		{
			return -1;
		}
		
		return cmd_args->argc[ cmd_args->nesting ];
	}

	const char* CmdArgs_ArgV( int index )
	{
		CmdArgs* cmd_args = CmdArgs_GetCmdArgs();

		if( cmd_args == NULL )
		{
			return "";
		}
		
		return cmd_args->argv[ cmd_args->nesting ][ index ];
	}

	int CmdArgs_LocalClientNum( void )
	{
		CmdArgs* cmd_args = CmdArgs_GetCmdArgs();

		if( cmd_args == NULL )
		{
			return -1;
		}
		
		return cmd_args->localClientNum[ cmd_args->nesting ];
	}

	int CmdArgs_ControllerIndex( void )
	{
		CmdArgs* cmd_args = CmdArgs_GetCmdArgs();

		if( cmd_args == NULL )
		{
			return -1;
		}
		
		return cmd_args->controllerIndex[ cmd_args->nesting ];
	}

	void Cmd_AddCommandInternal( const char* cmdName, void (*function)(), cmd_function_s *allocedCmd )
	{
		libpsutil::symbol<void( const char *cmdName, void (*function)(), cmd_function_s *allocedCmd )> Cmd_AddCommandInternal{ 0x2B2EC0 };
		Cmd_AddCommandInternal( cmdName, function, allocedCmd );
	}
}