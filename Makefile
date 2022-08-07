prpr:
	cd prpr  && make

9cc:
	cd 9cc && make

prpr3: 9cc prpr
	sh prtest.sh ${arch} test1
	sh prtest.sh ${arch} test2
	sh prtest.sh ${arch} test3
	cd 9cc && make test arch=${arch}
	sh diff3.sh prpr

9cc3: 9cc prpr
	sh cctest.sh ${arch} test1
	sh cctest.sh ${arch} test2
	sh cctest.sh ${arch} test3
	cd 9cc && make test arch=${arch}
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
