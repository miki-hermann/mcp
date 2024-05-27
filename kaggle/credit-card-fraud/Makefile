.PHONY: all 2023 equal after guess clean scratch

all: 2023

creditcard_2023.csv: creditcard_2023.csv.zip
	@echo "Unzipping creditcard_2023.csv.zip file. It will take a while ..."
	unzip creditcard_2023.csv.zip

2023: creditcard_2023.csv creditcard_2023.txt
	mcp-overview -i creditcard_2023.csv -c 30
	mcp-sample   -i creditcard_2023.csv  -W 2.5%
	mcp-trans    -i creditcard_2023_sample.csv -m creditcard_2023.txt
	mcp-uniq     -i creditcard_2023_sample.mat
	mcp-pthread  -i creditcard_2023_sample.unq -o creditcard_2023.out -l creditcard_2023 -d random
	mcp-trans    -i creditcard_2023.csv -m creditcard_2023.txt
	mcp-uniq     -i creditcard_2023.mat
	mcp-check    -i creditcard_2023.unq -l creditcard_2023_NOT_fraudulent.log -o creditcard_2023_NOT_fraudulent_uniq.out
	mcp-check    -i creditcard_2023.unq -l creditcard_2023_FRAUDULENT.log     -o creditcard_2023_FRAUDULENT_uniq.out
	mcp-check    -i creditcard_2023.mat -l creditcard_2023_NOT_fraudulent.log -o creditcard_2023_NOT_fraudulent_mat.out
	mcp-check    -i creditcard_2023.mat -l creditcard_2023_FRAUDULENT.log     -o creditcard_2023_FRAUDULENT_mat.out

equal: creditcard_2023.csv creditcard_2023.txt cc23_F.cln cc23_NF.cln
	mcp-overview -i creditcard_2023.csv -c 30
	mcp-clean   -i  creditcard_2023.csv -m cc23_F.cln  -o cc23_F.csv
	mcp-clean   -i  creditcard_2023.csv -m cc23_NF.cln -o cc23_NF.csv
	mcp-sample  -i cc23_F.csv  -W 2%
	mcp-sample  -i cc23_NF.csv -W 2%
	cat cc23_F_sample.csv cc23_NF_sample.csv > cc23.csv
	mcp-trans   -i cc23.csv -m creditcard_2023.txt
	mcp-uniq    -i cc23.mat
	mcp-pthread -i cc23.unq -o cc23.out -l cc23 -d random
	mcp-trans   -i creditcard_2023.csv -m creditcard_2023.txt
	mcp-uniq    -i creditcard_2023.mat
	mcp-check   -i creditcard_2023.unq -l cc23_NOT_fraudulent.log -o cc23_NOT_fraudulent_uniq.out
	mcp-check   -i creditcard_2023.unq -l cc23_FRAUDULENT.log     -o cc23_FRAUDULENT_uniq.out
	mcp-check   -i creditcard_2023.mat -l cc23_NOT_fraudulent.log -o cc23_NOT_fraudulent_mat.out
	mcp-check   -i creditcard_2023.mat -l cc23_FRAUDULENT.log     -o cc23_FRAUDULENT_mat.out

after: creditcard_2023.csv creditcard_2023.txt
	mcp-overview -i creditcard_2023.csv -c 30
	mcp-trans    -i creditcard_2023.csv -m creditcard_2023.txt
	mcp-uniq     -i creditcard_2023.mat
	mcp-sample   -i creditcard_2023.unq -W 2.5%
	mcp-pthread  -i creditcard_2023_sample.unq -hdr creditcard_2023.hdr -o creditcard_2023.out -l creditcard_2023 -d random
	mcp-check    -i creditcard_2023.unq -l creditcard_2023_NOT_fraudulent.log -o creditcard_2023_NOT_fraudulent_uniq.out
	mcp-check    -i creditcard_2023.unq -l creditcard_2023_FRAUDULENT.log     -o creditcard_2023_FRAUDULENT_uniq.out
	mcp-check    -i creditcard_2023.mat -l creditcard_2023_NOT_fraudulent.log -o creditcard_2023_NOT_fraudulent_mat.out
	mcp-check    -i creditcard_2023.mat -l creditcard_2023_FRAUDULENT.log     -o creditcard_2023_FRAUDULENT_mat.out

#-------------------------------------------------------------------------------

guess: creditcard_2023.csv creditcard.name
	mcp-guess   -i creditcard_2023.csv -n creditcard.name -o dummy_2023.txt

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt *.hdr
	rm -f *-overview.*
	rm -f cc23.csv cc23_F.csv cc23_NF.csv

scratch: clean
	rm -f *~
	rm -f dummy_*.txt
	rm -f *_sample*
	rm -f *_clean*
	rm -f /tmp/mcp-tmp-*.txt
	rm -f creditcard_2023.csv
