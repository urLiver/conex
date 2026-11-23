conex, short for config extender is a project aiming to make .cfg modding easer.

### Supported Games:
- Ghost
- Modern Warfare 3

### How to install
- Download Release and place content of tmp into your tmp folder on the PS3

#### Ui:
If you are in a game you may press L3 to open and close the dvar and command viewer.

#### Commands:

##### For Dumping
- dump_commands
- dump_dvarsimple, dumps them with a easier formatting
- dump_dvar

##### Misc
- vstr <variable: string>
- iprintln <message: string>
- iprintlnbold <message: string
- execfromdisk <filepath: string> 

##### Bindings
- bind_cmd <key: string> <command: string>
- unbind_cmd <key: string>
- old binding commands such as +melee if the game loads them

##### Host only, applying only to the users too
- god
- demigod
- notarget
- ufo
- noclip
- give <internal weapon name: string>

##### Account
- xuid ( Only applies to MW3 Mp)

#### Custom Dvars:
- unlock_dlc_content: as the name says, works partially depending on the game
- unlock_any_attachment_combo: no attachment combo limit

### How to build
Setup PS3 Sdk and so on, Open Project in Visual Studio build each Part

### What you need for the Build:
- PS3 SDK 4.75
- [libpsutil ( i used Version 1.0.5 )](https://github.com/skiff/libpsutil)

## Credits
- A Good Friend
- urLiver

Enjoy :D