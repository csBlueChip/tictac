#ifndef  ANSI_H_
#define  ANSI_H_

//-----------------------------------------------------------------------------
// internal definitions
//
#define  C_BLK     (0)
#define  C_BLU     (4)
#define  C_RED     (1)
#define  C_MAG     (5)
#define  C_GRN     (2)
#define  C_CYN     (6)
#define  C_YEL     (3)
#define  C_WHT     (7)

#define  NORM      (0)

#define  BRT       (100)
#define  DIM       (200)

#define  INK(n)    (30+(n))
#define  PAPER(n)  (40+(n))

//-----------------------------------------------------------------------------
// User colours

// 5 shades
#define   BLK      (    INK(C_BLK))
#define  DGRY      (BRT+INK(C_BLK))

#define  LGRY      (DIM+INK(C_WHT))
#define   WHT      (    INK(C_WHT))
#define  BWHT      (BRT+INK(C_WHT))

// By Luminance: Blu, Red, Mag, Grn, Cyn, Yel
#define  DBLU      (DIM+INK(C_BLU))
#define   BLU      (    INK(C_BLU))
#define  BBLU      (BRT+INK(C_BLU))

#define  DRED      (DIM+INK(C_RED))
#define   RED      (    INK(C_RED))
#define  BRED      (BRT+INK(C_RED))

#define  DMAG      (DIM+INK(C_MAG))
#define   MAG      (    INK(C_MAG))
#define  BMAG      (BRT+INK(C_MAG))

#define  DGRN      (DIM+INK(C_GRN))
#define   GRN      (    INK(C_GRN))
#define  BGRN      (BRT+INK(C_GRN))

#define  DCYN      (DIM+INK(C_CYN))
#define   CYN      (    INK(C_CYN))
#define  BCYN      (BRT+INK(C_CYN))

#define  DYEL      (DIM+INK(C_YEL))
#define   BRN      (DYEL)
#define   YEL      (    INK(C_YEL))
#define  BYEL      (BRT+INK(C_YEL))

//-----------------------------------------------------------------------------
#define  ONBLK     (PAPER(C_BLK))
#define  ONBLU     (PAPER(C_BLU))
#define  ONRED     (PAPER(C_RED))
#define  ONMAG     (PAPER(C_MAG))
#define  ONGRN     (PAPER(C_GRN))
#define  ONCYN     (PAPER(C_CYN))
#define  ONYEL     (PAPER(C_YEL))
#define  ONWHT     (PAPER(C_WHT))

//-----------------------------------------------------------------------------
#include  <stdio.h>

//+============================================================================
// Set ink colour - use "user colours" [above]
//
static inline  void  ink   (int c)  {  printf("\e[0;%d;%dm", c/100, c%100);  }
static inline  void  paper (int c)  {  printf("\e[%dm", c);  }

//+============================================================================
// Cursor to Top-Left {1,1}
//
static inline  void  home (void)    {  printf("\e[H");  }

//+============================================================================
// Clear Screen & Cursor to Top-Left
//
static inline  void  cls (void)     {  printf("\e[2J");  home();  }

//+============================================================================
// Show/Hide cursor
//
static inline  void  curOn (void)   {  printf("\e[?25h");  }
static inline  void  curOff (void)  {  printf("\e[?25l");  }

//+============================================================================
// Move cursor to {y,x} ...Top-left is {1,1}
//
static inline  void  goyx (int y,  int x)  {  printf("\e[%d;%dH", y, x);  }

#endif //ANSI_H_
