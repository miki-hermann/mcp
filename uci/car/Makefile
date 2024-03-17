all:
	mcp-trans   -i car.data -m car.txt
	mcp-uniq    -i car.mat
	mcp-split   -i car.unq  --ratio 49
	mcp-seq     -i car.lrn  -o car-hrn.out   --formula car-hrn
	mcp-seq     -i car.lrn  -o car-bij.out   --formula car-bij  --closure bij
	mcp-check   -i car.chk  -l car-hrn_vgood.log  -o car-hrn_vgood.out
	mcp-check   -i car.chk  -l car-bij_vgood.log  -o car-bij_vgood.out  --print clause

.PHONY: guess clean scratch

guess:
	mcp-guess   -i car.data -n car.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
	rm -f dummy.txt
