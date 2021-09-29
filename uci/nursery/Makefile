all:
	mcp-trans   -i nursery.data -m nursery.txt
	mcp-uniq    -i nursery.mat
	mcp-split   -i nursery.unq  --ratio 20
	mcp-pthread -i nursery.lrn  -o nursery-bij.out            --formula nursery-bij --closure bij
	mcp-check   -i nursery.chk  -l nursery-bij_spec_prior.log -o nursery-bij_spec_prior.out
	mcp-check   -i nursery.chk  -l nursery-bij_not_recom.log  -o nursery-bij_not_recom.out
#	mcp-pthread -i nursery.lrn  -o nursery-cnf.out            --formula nursery-cnf --closure cnf
#	mcp-check   -i nursery.chk  -l nursery-cnf_spec_prior.log -o nursery-cnf_spec_prior.out
#	mcp-check   -i nursery.chk  -l nursery-cnf_not_recom.log  -o nursery-cnf_not_recom.out

.PHONY: guess clean scratch

guess:
	mcp-guess   -i nursery.data -n nursery.names -o dummy.txt

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy.txt
