// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.
// Screen edges
	@16384
	D = A
	@LSB
	M = D
	@INDEX
	M = D

	(LOOP)

	@KBD
	D = M
	@PRESSED
	D;JNE
	@NOTPRESSED
	0;JMP	

(PRESSED)
	@INDEX
	D = M

	@SCREEN
	A = D
	M = -1

	@256
	D = A
	@INDEX
	M = M + D

	@LOOP
	0;JMP

(NOTPRESSED)
	@INDEX
	D = M
	@LOOP
	D;JLE

	@SCREEN
	A = D
	M = 1

	@256
	D = A
	@INDEX
	M = M - D

	@LOOP
	0;JMP

(END)
	@END
	0;JMP
