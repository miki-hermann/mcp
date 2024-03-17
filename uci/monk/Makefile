all: 1 2 3

.PHONY: 1 2 3 guess clean scratch

1:
	mcp-trans -i monks-1.train -m monks.txt -o monks.mat
	mcp-seq   -i monks.mat -o monks-1.out -m hide -l monks-1
	mcp-trans -i monks-1.test  -m monks.txt -o monks.mat
	mcp-check -i monks.mat -l monks-1_1.log -o monks-1_1.out

2:
	mcp-trans -i monks-2.train -m monks.txt -o monks.mat
	mcp-seq   -i monks.mat -o monks-2.out -m hide --closure cnf -l monks-2
	mcp-trans -i monks-2.test  -m monks.txt -o monks.mat
	mcp-check -i monks.mat -l monks-2_1.log -o monks-2_1.out --print clause

3:
	mcp-trans -i monks-3.train -m monks.txt -o monks.mat
	mcp-seq   -i monks.mat -o monks-3.out -m hide --print mix --closure bij -l monks-3
	mcp-trans -i monks-3.test  -m monks.txt -o monks.mat
	mcp-check -i monks.mat -l monks-3_1.log -o monks-3_1.out --print mix

guess:
	mcp-guess   -i monks-1.train     -n monks.names     -o dummy-new.txt
	mcp-guess   -i old-monks-1.train -n old-monks.names -o dummy-old.txt

clean:
	rm -f *.mat *.hdr *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy-*.txt
