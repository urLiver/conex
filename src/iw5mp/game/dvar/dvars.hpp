#pragma once

#include <inttypes.h>

namespace Game
{
    union DvarValue
    {
        bool enabled;
        int integer;
        unsigned int unsignedInt;
        float value;
        float vec[4];
        const char *string;
        uint8_t color[4];
    };

    struct DvarLimits_unnamed_typevec
    {
        float min;
        float max;
    };

    struct DvarLimits_unnamed_type_integer
    {
        int min;
        int max;
    };

    struct DvarLimits_unnamed_type_enumeration
    {
        int stringCount;
        const char *const *strings;
    };

    struct DvarLimits_unnamed_type_value
    {
        float min;
        float max;
    };

    union DvarLimits
    {
        DvarLimits_unnamed_type_enumeration enumeration;
        DvarLimits_unnamed_type_integer integer;
        DvarLimits_unnamed_type_value value;
        DvarLimits_unnamed_typevec vec;
    };

    struct dvar_t
    {
        const char *name;
        uint16_t flags;
        uint8_t type;
        uint8_t modified;
        DvarValue current;
        DvarValue latched;
        DvarValue reset;
        DvarLimits domain;
        dvar_t *hashNext;
    };

    dvar_t* Dvar_FindVar( const char *dvarName );
    
    dvar_t* Dvar_RegisterBool( const char *dvarName, bool value, unsigned int flags );

    void Dvar_ForEach( void (*callback)(dvar_t *, void *), void *userData );

    bool Dvar_GetBool( const char* dvarName );

    const char* Dvar_ValueToString( dvar_t* dvar );

    void Dvar_DumpAllDvars( void );

    void Dvar_DumpAllDvarsSimple( void );
}