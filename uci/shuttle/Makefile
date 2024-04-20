.PHONY: all perl clean scratch

all:
	mcp-overview -i shuttle.trn -o shuttle-trn-overview.txt -c 9
	mcp-overview -i shuttle.tst -o shuttle-tst-overview.txt -c 9
	mcp-trans -i shuttle.trn -m shuttle.txt -o shuttle.lrn
	mcp-trans -i shuttle.tst -m shuttle.txt -o shuttle.chk
	# mcp-seq -i shuttle.lrn -o shuttle-hrn.out -l shuttle-hrn --action select 1
	# mcp-check -i shuttle.chk -l shuttle-hrn_1.log -o shuttle-hrn_1.out
	mpirun mcp-mpi -i shuttle.lrn -o shuttle-bij.out -l shuttle-bij --closure bij
	# mcp-check -i shuttle.chk -l shuttle-bij_2.log -o shuttle-bij_2.out
	# Testing bypass
	mcp-check -i shuttle.chk -l shuttle-bij_5.log -o shuttle-bij_5.out
	# mcp-check -i shuttle.chk -l shuttle-bij_6.log -o shuttle-bij_6.out

perl: shuttle.trn shuttle.pl
	shuttle.pl < shuttle.trn > shuttle.csv

clean:
	rm -f *.mat *.hdr *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f *.csv
