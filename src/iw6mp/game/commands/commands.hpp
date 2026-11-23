#pragma once

#include <libpsutil.h>

namespace Game
{
    extern bool key_toggled_table[ 4 ][ 128 ]; 

    extern std::string key_cmd_table[ 4 ][ 128 ]; 
    
    void SetupCommands();
}