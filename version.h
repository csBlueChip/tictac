#ifndef  VERSION_H_
#define  VERSION_H_

// 0.7.0 - first release .. AI incomplete
// 0.8.0 - full screen selection (with mouse)
// 0.9.0 - now with basic AI
// 0.9.5 - The bots are getting there slowly...
// 0.9.6 - Various interface tweaks, incl "-?" for help

#define  TOOLNAMES  "TicTac"

#define  VER_MAJ    0
#define  VER_MIN    9
#define  VER_SUB    6

#define  STR_(x)    #x
#define  STR(x)     STR_(x)

#define  VER_STR    "Version " STR(VER_MAJ) "." STR(VER_MIN) "." STR(VER_SUB)

#define  AUTHOR     "\e[0;1;32mBlue\e[0;1;34mChip\e[0m"
#define  DATE       "July/2025"

#endif //VERSION_H_

