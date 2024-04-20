all: setosa versicolor virginica # perl

iris.lrn: iris.data iris.txt
	mcp-overview -i iris.data -c 4
	mcp-trans -i iris.data -m iris.txt
	mcp-uniq  -i iris.mat
	mcp-split -i iris.unq  --ratio 49

common: iris.lrn
	mcp-seq   -i iris.lrn  -o iris-hrn.out --formula iris-hrn
	mcp-seq   -i iris.lrn  -o iris-bij.out --formula iris-bij  --closure bij
	mcp-seq   -i iris.lrn  -o iris-cnf.out --formula iris-cnf  --closure cnf

setosa: common
	mcp-check -i iris.chk  -l iris-hrn_setosa.log     -o iris-hrn_setosa.out
	mcp-check -i iris.chk  -l iris-bij_setosa.log     -o iris-bij_setosa.out
	mcp-check -i iris.chk  -l iris-cnf_setosa.log     -o iris-cnf_setosa.out

versicolor: common
	mcp-check -i iris.chk  -l iris-hrn_versicolor.log -o iris-hrn_versicolor.out
	mcp-check -i iris.chk  -l iris-bij_versicolor.log -o iris-bij_versicolor.out
	mcp-check -i iris.chk  -l iris-cnf_versicolor.log -o iris-cnf_versicolor.out

virginica: common
	mcp-check -i iris.chk  -l iris-hrn_virginica.log  -o iris-hrn_virginica.out
	mcp-check -i iris.chk  -l iris-bij_virginica.log  -o iris-bij_virginica.out
	mcp-check -i iris.chk  -l iris-cnf_virginica.log  -o iris-cnf_virginica.out


.PHONY: guess clean scratch perl

guess:
	mcp-guess -i iris.data -n iris.names -o dummy.txt

perl: iris.pl iris.data
	iris.pl < iris.data > iris.csv

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
