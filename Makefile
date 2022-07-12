prpr:
	cd prpr  && make

9cc:
	cd 9cc && make

prpr1: 9cc prpr
	./prtest.sh

prpr3: prpr1
	./prtest.sh
	./prtest.sh
	./prtest.sh
	cd 9cc && make test

9cc1: 9cc prpr
	./cctest.sh

9cc3: 9cc1
	./cctest.sh
	./cctest.sh
	./cctest.sh
	cd 9cc && make test

re:
	cd prpr && make re
	cd 9cc && make re

.PHONY: prpr 9cc prpr1 9cc1 re
