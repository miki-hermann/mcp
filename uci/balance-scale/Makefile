all:
	mcp-trans -m bsc.txt -i bsc.data
	mcp-split -i bsc.mat --ratio 5
	mpirun    -np 3 mcp-mpi -i bsc.lrn -o bsc-mpi.out -l bsc
	mcp-check -i bsc.chk -l bsc_B.log -o bsc_B.out
	mcp-check -i bsc.chk -l bsc_L.log -o bsc_L.out
	mcp-check -i bsc.chk -l bsc_R.log -o bsc_R.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
