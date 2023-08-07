# TerminalBasedFileExplorer

Normal Mode
1.I am opening the files in vi editor.All workings of vi remain standard.

2.Resizing is handled only vertically.After change in terminal height it is triggered only after some action inside the normal mode (directory change-h/backspace/enter/left arrow/rigth arrow).Unable to implement trigger via signal() as my display function in normal mode takes in a parameter.


Command mode
1.Due to Esc and arrow keys(up,down,right,left)  having the same first byte, the usage of these keys will make you exit command mode and take you to the normal mode.

3.Type the command and press enter to execute them in command mode.

2.Incorrectly typed commands will also be handled after pressing enter.Backspace can be used to clear character.

3.The path provided as arguments to the various commands are assumed to be
	1.Absolute (starting from root)
	2.Relative wrt the current directory 
	
4.Tidle is  handled as argument only for goto command.

