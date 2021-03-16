all: setosa versicolor virginica

iris.lrn: iris.data iris.txt
	mcp-trans -i iris.data -m iris.txt
	mcp-split -i iris.mat  --ratio 49

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


.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
