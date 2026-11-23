#include "write.hpp"


#include "../game/utils/utils.hpp"

#include <libpsutil.h>
#include <cell/cell_fs.h>

void WriteFile( const char* path, const char* fmt, ... )
{
    char dest[ 512 ] = { 0 };

    va_list vargs;

    va_start( vargs, fmt );

    Game::Com_vsnprintf( dest, sizeof( dest ), fmt, vargs );

    int file_descriptor = 0;

	CellFsErrno error = cellFsOpen( path, CELL_FS_O_RDWR | CELL_FS_O_CREAT | CELL_FS_O_APPEND, &file_descriptor, NULL, 0 );

	if ( error != CELL_FS_SUCCEEDED )
	{
        LogWrite( "WriteFile: Failed cellFsOpen" );

        goto end;
	}

	error = cellFsWrite( file_descriptor, dest, strlen( dest ), NULL );
    
	if ( error != CELL_FS_SUCCEEDED )
	{
        LogWrite( "WriteFile: Failed cellFsWrite" );

        goto end;
	}

end:
	cellFsClose( file_descriptor );

    va_end( vargs );
}

void LogWrite( const char* fmt, ... )
{
    char dest[ 512 ] = { 0 };

    va_list vargs;

    va_start( vargs, fmt );

    Game::Com_vsnprintf( dest, sizeof( dest ), fmt, vargs );
    
    size_t len = strlen( dest );
    if( len > 0 && dest[ len - 1 ] != '\n' )
    {
        if( len < sizeof( dest ) - 1 )
        {
            dest[ len ] = '\n';
            dest[ len + 1 ] = '\0';
        }
        else
        {
            dest[ sizeof( dest ) - 2 ] = '\n';
            dest[ sizeof( dest ) - 1 ] = '\0';
        }
    }

    int file_descriptor = 0;

	CellFsErrno error = cellFsOpen( "/dev_hdd0/tmp/conex/data/iw5mp_log.txt", CELL_FS_O_RDWR | CELL_FS_O_CREAT | CELL_FS_O_APPEND, &file_descriptor, NULL, 0 );

	if( error != CELL_FS_SUCCEEDED )
	{
        va_end( vargs );
    
        return;
	}

	cellFsWrite( file_descriptor, dest, strlen( dest ), NULL );
	
    cellFsClose( file_descriptor );

    va_end( vargs );
}
