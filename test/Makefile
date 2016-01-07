.PHONY: all test test_pic test_score clean

all: test

test: test_pic test_score

clean:
	rm -f testfilt.pic test.pic test.oct test.rad

test_pic: testfilt.png

test_score: testscore.val

testscore_rad.txt: test.oct testscore_grid.pts Makefile
	cat testscore_grid.pts | rtrace -ab 10 -h test.oct > testscore_rad.txt || rm testscore_rad.txt

testscore_grid.pts: Makefile
	R --vanilla --slave -e 'write.table(expand.grid(x = 545 + (300-1)*seq(-1, 1, length.out = 20), y = 0, z = 0 + (325-1)*seq(-1, 1, length.out = 20), xv = 0, yv = 1, zv = 0), row.names = FALSE, col.names = FALSE, file = "testscore_grid.pts")'

testscore.val: testscore_rad.txt
	R --vanilla --slave -e 'values = read.table("testscore_rad.txt", header = FALSE)[,1]; cat(min(values), "\t", (max(values) - min(values)) / min(values), "\n")' > testscore.val || rm testscore.val

testfilt.png: testfilt.pic
	convert testfilt.pic testfilt.png

testfilt.pic: test.pic Makefile
	pfilt -x /2 -y /2 test.pic > testfilt.pic || rm testfilt.pic

test.pic: test.oct Makefile
	rpict -x 500 -y 500 -vp 545 -1000 0 -vd 0 1 0 -ab 10 test.oct > test.pic || rm test.pic

test.oct: test.rad
	oconv test.rad > test.oct || rm test.oct

test.rad: test.rad.m4 Makefile
	m4 -Da=200 -Db=500 -Dc=200 -Dd=100 -De=200 -Df=200 -Di=100 -Dg=100 -Dh=250 -Dj=false -Dk=true test.rad.m4 > test.rad || rm test.rad
