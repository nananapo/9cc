prpr:
	cd prpr  && make

9cc:
	cd 9cc && make

prpr1: 9cc prpr
	./prtest.sh

prpr3: 9cc prpr
	./prtest1.sh
	./prtest2.sh
	./prtest3.sh
	cd 9cc && make test
	./diffprpr.sh

9cc1: 9cc prpr
	./cctest1.sh

9cc3: 9cc prpr
	./cctest1.sh
	./cctest2.sh
	./cctest3.sh
	cd 9cc && make test
	./diffcc.sh

re:
	cd prpr && make re
	cd 9cc && make re

.PHONY: prpr 9cc prpr1 9cc1 re
