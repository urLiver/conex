#include "dvars.hpp"

#include "../utils/utils.hpp"

#include "../../fs/write.hpp"

#include <libpsutil.h>

namespace Game
{
    dvar_t* Dvar_FindVar( const char *dvarName )
    {
        static libpsutil::symbol<dvar_t*( const char *dvarName )> Dvar_FindVar{ 0x50A63C };
        return Dvar_FindVar( dvarName );
    }
    
    dvar_t* Dvar_RegisterBool( const char *dvarName, bool value, unsigned int flags )
    {
        static libpsutil::symbol<dvar_t*( const char *dvarName, bool value, unsigned int flags )> Dvar_RegisterBool{ 0x6BDAB8 };
        return Dvar_RegisterBool( dvarName, value, flags );
    }

    void Dvar_ForEach( void (*callback)(dvar_t *, void *), void *userData )
    {
        static libpsutil::symbol<void( void (*callback)(dvar_t *, void *), void *userData )> Dvar_ForEach{ 0x57B198 };
        Dvar_ForEach( callback, userData );
    }

    bool Dvar_GetBool( const char* dvarName )
    {
        static libpsutil::symbol<bool( const char* dvarName )> Dvar_GetBool{ 0x5D4B8C };
        return Dvar_GetBool( dvarName );
    }
    
    void Dvar_SetIntByName( const char* dvarName, int value )
    {
        static libpsutil::symbol<void( const char* dvarName, int value )> Dvar_SetIntByName{ 0x6C19C8 };
        Dvar_SetIntByName( dvarName, value );
    }

    char vs[ 64 ];
    const char* Dvar_ValueToString( dvar_t* dvar )
    {
        static libpsutil::symbol<int( char *__str, size_t __size, const char *__format, ... )> Com_sprintf{ 0x38CFAC };

        DvarValue value = dvar->current;

        Com_sprintf( vs, sizeof( vs ), "%s", "(null)" );

        switch ( dvar->type )
        {
            case 0:
                Com_sprintf( vs, sizeof( vs ), "%s", value.enabled ? "true" : "false" );
                break;
            case 1:
                Com_sprintf( vs, sizeof( vs ), "%f", value.value, value.value );
                break;
            case 2:
                Com_sprintf( vs, sizeof( vs ), "%f %f", value.vec[ 0 ], value.vec[ 1 ] );
                break;
            case 3:
                Com_sprintf( vs, sizeof( vs ), "%f %f %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ] );
                break;
            case 4:
                Com_sprintf( vs, sizeof( vs ), "%f %f %f %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ], value.vec[ 3 ] );
                break;
            case 5:
                Com_sprintf( vs, sizeof( vs ), "%i", value.unsignedInt );
                break;
            case 6:
                if ( dvar->domain.enumeration.stringCount )
                {
                    Com_sprintf( vs, sizeof( vs ), "%s", dvar->domain.enumeration.strings[ value.integer ] );
                }
                break;
            case 7:
                Com_sprintf( vs, sizeof( vs ), "%s", value.string );
                break;
            case 8:
                Com_sprintf( vs, sizeof( vs ), "%i %i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ], value.color[ 3 ] );
                break;
            case 9:
                Com_sprintf( vs, sizeof( vs ), "%i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ] );
                break;
        }

        return vs;
    }
    
    char dvs[ 256 ];
    const char* Dvar_InfoString( dvar_t* dvar )
    {
        static libpsutil::symbol<int( char *__str, size_t __size, const char *__format, ... )> Com_sprintf{ 0x38CFAC };
        
        DvarValue value = dvar->current;
        DvarLimits domain = dvar->domain;

        switch ( dvar->type )
        {
            case 0:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Bool\n\tValue: %s", value.enabled ? "true" : "false" );
                break;
            case 1:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Float\n\tValue: %f\n\tMin: %f\n\tMax: %f", value.value, domain.value.min, domain.value.max );
                break;
            case 2:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Vec 2\n\tValue: %f %f\n\tMin: %f\n\tMax: %f", value.vec[ 0 ], value.vec[ 1 ], domain.vec.min, domain.vec.max );
                break;
            case 3:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Vec 3\n\tValue: %f %f %f\n\tMin: %f\n\tMax: %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ], domain.vec.min, domain.vec.max );
                break;
            case 4:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Vec 4\n\tValue: %f %f %f %f\n\tMin: %f\n\tMax: %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ], value.vec[ 3 ], domain.vec.min, domain.vec.max );
                break;
            case 5:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Int\n\tValue Unsigned: %i\n\tValue Signed: %i\n\tMin: %i\n\tMax: %i", value.unsignedInt, value.integer, domain.integer.min, domain.integer.max );
                break;
            case 6:
                if ( dvar->domain.enumeration.stringCount )
                {
                    Com_sprintf( dvs, sizeof( dvs ), "\tType: Enum\n\tValue: %s", dvar->domain.enumeration.strings[ value.integer ] );

                    for( int i = 0; i < domain.enumeration.stringCount; i++ )
                    {
                        Com_sprintf( dvs, sizeof( dvs ), "%s\n\t\t%s", dvs, dvar->domain.enumeration.strings[ i ] );
                    }
                }
                break;
            case 7:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: String\n\tValue: %s", value.string );
                break;
            case 8:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Color 4\n\tValue: %i %i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ], value.color[ 3 ] );
                break;
            case 9:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Color 3\n\tValue: %i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ] );
                break;
            default:
                Com_sprintf( dvs, sizeof( dvs ), "\tType: Unknown\n\tValue: %s", "(null)" );
            break;
        }

        return dvs;
    }
    
    const char* Dvar_InfoStringSimple( dvar_t* dvar )
    {
        static libpsutil::symbol<int( char *__str, size_t __size, const char *__format, ... )> Com_sprintf{ 0x38CFAC };
        
        DvarValue value = dvar->current;
        DvarLimits domain = dvar->domain;

        switch ( dvar->type )
        {
            case 0:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Bool; Value: %s", value.enabled ? "true" : "false" );
                break;
            case 1:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Float; Value: %f; Min: %f; Max: %f", value.value, domain.value.min, domain.value.max );
                break;
            case 2:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Vec 2; Value: %f %f; Min: %f; Max: %f", value.vec[ 0 ], value.vec[ 1 ], domain.vec.min, domain.vec.max );
                break;
            case 3:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Vec 3; Value: %f %f %f; Min: %f; Max: %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ], domain.vec.min, domain.vec.max );
                break;
            case 4:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Vec 4; Value: %f %f %f %f; Min: %f; Max: %f", value.vec[ 0 ], value.vec[ 1 ], value.vec[ 2 ], value.vec[ 3 ], domain.vec.min, domain.vec.max );
                break;
            case 5:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Int; Value Unsigned: %i; Value Signed: %i; Min: %i; Max: %i", value.unsignedInt, value.integer, domain.integer.min, domain.integer.max );
                break;
            case 6:
                if ( dvar->domain.enumeration.stringCount )
                {
                    Com_sprintf( dvs, sizeof( dvs ), "Type: Enum; Value: %s", dvar->domain.enumeration.strings[ value.integer ] );

                    for( int i = 0; i < domain.enumeration.stringCount; i++ )
                    {
                        Com_sprintf( dvs, sizeof( dvs ), "%s; %s", dvs, dvar->domain.enumeration.strings[ i ] );
                    }
                }
                break;
            case 7:
                Com_sprintf( dvs, sizeof( dvs ), "Type: String; Value: %s", value.string );
                break;
            case 8:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Color 4; Value: %i %i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ], value.color[ 3 ] );
                break;
            case 9:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Color 3; Value: %i %i %i", value.color[ 0 ], value.color[ 1 ], value.color[ 2 ] );
                break;
            default:
                Com_sprintf( dvs, sizeof( dvs ), "Type: Unknown; Value: %s", "(null)" );
            break;
        }

        return dvs;
    }

    void Dvar_DumpDvar( dvar_t* dvar, void* data )
    {
        if( dvar == NULL )
        {
            return;
        }

        WriteFile( "/dev_hdd0/tmp/conex/data/iw6mp_dvars.txt", "%s\n%s\n\n", dvar->name, Dvar_InfoString( dvar ) );
    }

    void Dvar_DumpAllDvars( void )
    {
        Dvar_ForEach( Dvar_DumpDvar, NULL );
    }
    
    void Dvar_DumpDvarSimple( dvar_t* dvar, void* data )
    {
        if( dvar == NULL )
        {
            return;
        }

        WriteFile( "/dev_hdd0/tmp/conex/data/iw6mp_dvarssimple.txt", "%s\n%s\n\n", dvar->name, Dvar_InfoStringSimple( dvar ) );
    }

    void Dvar_DumpAllDvarsSimple( void )
    {
        Dvar_ForEach( Dvar_DumpDvarSimple, NULL );
    }
}