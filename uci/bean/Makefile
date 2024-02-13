.PHONY: all guess csv clean scratch

all: horn bij # cnf

horn: bean_horn.out
	mcp-check   -i bean.chk  -l bean_horn_BARBUNYA.log   -o bean_horn_BARBUNYA.out   --print clause
	mcp-check   -i bean.chk  -l bean_horn_BOMBAY.log     -o bean_horn_BOMBAY.out     --print clause
	mcp-check   -i bean.chk  -l bean_horn_CALI.log       -o bean_horn_CALI.out       --print clause
	mcp-check   -i bean.chk  -l bean_horn_DERMASON.log   -o bean_horn_DERMASON.out   --print clause
	mcp-check   -i bean.chk  -l bean_horn_HOROZ.log      -o bean_horn_HOROZ.out      --print clause
	mcp-check   -i bean.chk  -l bean_horn_SEKER.log      -o bean_horn_SEKER.out      --print clause
	mcp-check   -i bean.chk  -l bean_horn_SIRA.log       -o bean_horn_SIRA.out       --print clause

bean_horn.out: bean.lrn
	mcp-pthread  -i bean.lrn  -o bean_horn.out           --formula bean_horn         --closure horn

bij: bean_bij.out
	mcp-check   -i bean.chk  -l bean_bij_BARBUNYA.log    -o bean_bij_BARBUNYA.out    --print clause
	mcp-check   -i bean.chk  -l bean_bij_BOMBAY.log      -o bean_bij_BOMBAY.out      --print clause
	mcp-check   -i bean.chk  -l bean_bij_CALI.log        -o bean_bij_CALI.out        --print clause
	mcp-check   -i bean.chk  -l bean_bij_DERMASON.log    -o bean_bij_DERMASON.out    --print clause
	mcp-check   -i bean.chk  -l bean_bij_HOROZ.log       -o bean_bij_HOROZ.out       --print clause
	mcp-check   -i bean.chk  -l bean_bij_SEKER.log       -o bean_bij_SEKER.out       --print clause
	mcp-check   -i bean.chk  -l bean_bij_SIRA.log        -o bean_bij_SIRA.out        --print clause

bean_bij.out: bean.lrn
	mcp-pthread  -i bean.lrn  -o bean_bij.out            --formula bean_bij          --closure bij

cnf: bean_cnf.out
	mcp-check   -i bean.chk  -l bean_cnf_BARBUNYA.log    -o bean_cnf_BARBUNYA.out    --print clause
	mcp-check   -i bean.chk  -l bean_cnf_BOMBAY.log      -o bean_cnf_BOMBAY.out      --print clause
	mcp-check   -i bean.chk  -l bean_cnf_CALI.log        -o bean_cnf_CALI.out        --print clause
	mcp-check   -i bean.chk  -l bean_cnf_DERMASON.log    -o bean_cnf_DERMASON.out    --print clause
	mcp-check   -i bean.chk  -l bean_cnf_HOROZ.log       -o bean_cnf_HOROZ.out       --print clause
	mcp-check   -i bean.chk  -l bean_cnf_SEKER.log       -o bean_cnf_SEKER.out       --print clause
	mcp-check   -i bean.chk  -l bean_cnf_SIRA.log        -o bean_cnf_SIRA.out        --print clause

bean_cnf.out: bean.lrn
	mcp-pthread  -i bean.lrn  -o bean_cnf.out            --formula bean_cnf          --closure cnf

bean.lrn: bean.unq
	mcp-split   -i bean.unq  --ratio 20

bean.unq: bean.mat
	mcp-uniq    -i bean.mat

bean.mat: bean.data bean.txt
	mcp-trans   -i bean.data -m bean.txt

guess: bean.data
	mcp-guess   -i bean.data -n bean.nam -o dummy.txt

csv: bean.mat
	mcp-mat2csv < bean.mat > bean.csv

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt *.csv

scratch: clean
	rm -f *~
	rm -f dummy.txt
