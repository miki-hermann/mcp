all:
	mcp-trans      -i abalone.data -m abalone.txt
	mcp-uniq       -i abalone.mat
	mcp-split      -i abalone.unq  --ratio 25
	mpirun mcp-mpi -i abalone.lrn -o abalone.out     --formula abalone
	mpirun mcp-mpi -i abalone.lrn -o abalone-bij.out --formula abalone-bij \
				--closure bij
	mcp-check      -i abalone.chk -l abalone_24.log     -o abalone_24.out
	mcp-check      -i abalone.chk -l abalone-bij_25.log -o abalone-bij_25.out

.PHONY: guess clean scratch

guess:
	mcp-guess      -i abalone.data -n abalone.nam -o dummy.txt

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy.txt
