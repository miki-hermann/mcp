all:
	mcp-trans      -i abalone.data -m abalone.txt
	mcp-split      -i abalone.mat  --ratio 25
	mpirun mcp-mpi -i abalone.lrn -o abalone.out     --formula abalone
	mpirun mcp-mpi -i abalone.lrn -o abalone-bij.out --formula abalone-bij \
				--closure bij
	mcp-check      -i abalone.chk -l abalone_27.log     -o abalone_27.out
	mcp-check      -i abalone.chk -l abalone-bij_27.log -o abalone-bij_27.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
