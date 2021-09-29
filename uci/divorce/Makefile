all:
	mcp-trans -i divorce.csv -m divorce.txt
	mcp-split -i divorce.mat  --ratio 49
	mcp-seq   -i divorce.lrn -o divorce-hrn.out --formula divorce-hrn -d hcard
	mcp-seq   -i divorce.lrn -o divorce-bij.out --formula divorce-bij --closure bij -d hcard
	mcp-seq   -i divorce.lrn -o divorce-cnf.out --formula divorce-cnf --closure cnf -d hcard
	mcp-check -i divorce.chk -l divorce-hrn_yes.log -o divorce-hrn_yes.out
	mcp-check -i divorce.chk -l divorce-bij_yes.log -o divorce-bij_yes.out --print clause
	mcp-check -i divorce.chk -l divorce-cnf_yes.log -o divorce-cnf_yes.out --print clause
	mcp-check -i divorce.chk -l divorce-hrn_no.log  -o divorce-hrn_no.out
	mcp-check -i divorce.chk -l divorce-cnf_no.log  -o divorce-cnf_no.out --print clause

.PHONY: guess clean scratch

guess:
	mcp-guess -i divorce.csv -n divorce.names -o dummy.txt

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy.txt
