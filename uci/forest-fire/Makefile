all:
	mcp-trans      -i ff.csv -m ff.txt
	mcp-split      -i ff.mat -r 10
	mpirun mcp-mpi -i ff.lrn -o ff-hrn.out --formula ff-hrn
	mcp-seq        -i ff.lrn -o ff-bij.out --formula ff-bij --closure bij
	mcp-pthread    -i ff.lrn -o ff-cnf.out --formula ff-cnf --closure cnf
	mcp-check      -i ff.chk -l ff-hrn_sep.log -o ff-hrn_sep.out
	mcp-check      -i ff.chk -l ff-bij_jul.log -o ff-bij_jul.out
	mcp-check      -i ff.chk -l ff-cnf_aug.log -o ff-cnf_aug.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~

