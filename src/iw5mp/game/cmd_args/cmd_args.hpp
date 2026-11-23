#pragma once

namespace Game
{
	struct cmd_function_s
	{
		cmd_function_s *next;
		const char *name;
		void (*function)();
	};

	#pragma pack(push, 1)
	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argc[8];
		const char **argv[8];
	};
	#pragma pack(pop)

	CmdArgs* CmdArgs_GetCmdArgs( void );

	int CmdArgs_ArgC( void );

	const char* CmdArgs_ArgV( int index );

	int CmdArgs_LocalClientNum( void );

	int CmdArgs_ControllerIndex( void );

	void Cmd_AddCommandInternal( const char* cmdName, void (*function)(), cmd_function_s *allocedCmd );
}