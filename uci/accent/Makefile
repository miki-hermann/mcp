all:
	mcp-trans      -i accent.csv -m accent.txt
	mcp-uniq       -i accent.mat
	mcp-split      -i accent.unq  --ratio 25
	mpirun mcp-mpi -i accent.lrn -o accent.out     --formula accent
	mcp-check      -i accent.chk -l accent_ES.log  -o accent_ES.out
	mcp-check      -i accent.chk -l accent_FR.log  -o accent_FR.out
	mcp-check      -i accent.chk -l accent_GE.log  -o accent_GE.out
	mcp-check      -i accent.chk -l accent_IT.log  -o accent_IT.out
	mcp-check      -i accent.chk -l accent_UK.log  -o accent_UK.out
	mcp-check      -i accent.chk -l accent_US.log  -o accent_US.out

.PHONY: guess clean scratch

guess:
	mcp-guess      -i accent.csv -n accent.names -o dummy.txt

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy.txt
