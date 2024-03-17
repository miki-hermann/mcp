all:
	mcp-trans -i shuttle.trn -m shuttle.txt -o shuttle.lrn
	mcp-trans -i shuttle.tst -m shuttle.txt -o shuttle.chk
	# mcp-seq -i shuttle.lrn -o shuttle-hrn.out -l shuttle-hrn --action select 1
	# mcp-check -i shuttle.chk -l shuttle-hrn_1.log -o shuttle-hrn_1.out
	mpirun mcp-mpi -i shuttle.lrn -o shuttle-bij.out -l shuttle-bij --closure bij
	mcp-check -i shuttle.chk -l shuttle-bij_1.log -o shuttle-bij_1.out
	mcp-check -i shuttle.chk -l shuttle-bij_4.log -o shuttle-bij_4.out
	mcp-check -i shuttle.chk -l shuttle-bij_5.log -o shuttle-bij_5.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.hdr *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
