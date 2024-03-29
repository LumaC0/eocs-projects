// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

// PsuedoCode
// mult = @R0
// Loop:
//   if (mult==0) goto END
//   R2 = R2 + R1
//   mult = mult - 1
//   goto loop
// END:

// Put your code here.
  @R2
  M = 0
//
  @sum
  M = 0
//
  @R0
  D = M
  @STOP
  D;JEQ
//
  @times
  M = D
  @STOP
  D;JEQ
//
  @R1
  D = M
  @STOP
  D;JEQ
//
  @R1
  D = M
  @mult
  M = D
//
(Loop)
  @mult
  D = M
  @sum
  M = M + D
  @times
  M = M - 1
  D = M
  @LOOP
  D;JGT
(STOP)
  @sum
  D = M
  @R2
  M = D
(END)
  @END
  0;JMP

