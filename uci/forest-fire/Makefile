.PHONY: all guess clean scratch

all: ff.csv ff.txt
	mcp-overview -i ff.csv -c 2
	mcp-trans      -i ff.csv -m ff.txt
	mcp-uniq       -i ff.mat
	mcp-split      -i ff.unq -r 10
	mcp-pthread    -i ff.lrn -o ff-hrn.out --formula ff-hrn
	mcp-seq        -i ff.lrn -o ff-bij.out --formula ff-bij --closure bij
	mcp-pthread    -i ff.lrn -o ff-cnf.out --formula ff-cnf --closure cnf
	mcp-check      -i ff.chk -l ff-hrn_sep.log -o ff-hrn_sep.out
	mcp-check      -i ff.chk -l ff-bij_jul.log -o ff-bij_jul.out
	mcp-check      -i ff.chk -l ff-cnf_aug.log -o ff-cnf_aug.out

guess:
	mcp-guess      -i ff.csv -n ff.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.prc *.unq *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
