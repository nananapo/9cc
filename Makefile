prpr:
	cd prpr  && make

9cc:
	cd 9cc && make

prpr1: 9cc prpr
	sh prtest.sh test1

prpr3: 9cc prpr
	sh prtest.sh test1
	sh prtest.sh test2
	sh prtest.sh test3
	cd 9cc && make test
	sh diff3.sh prpr

9cc1: 9cc prpr
	sh cctest.sh test1

9cc3: 9cc prpr
	sh cctest.sh test1
	sh cctest.sh test2
	sh cctest.sh test3
	cd 9cc && make test
	sh diff3.sh 9cc

re:
	cd prpr && make re
	cd 9cc && make re

clean:
	cd prpr && make clean
	cd 9cc && make clean

fclean:
	cd prpr && make fclean
	cd 9cc && make fclean

.PHONY: prpr 9cc prpr1 9cc1 re
