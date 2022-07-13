DIR1="9cc/test1/"
DIR2="9cc/test2/"
DIR3="9cc/test3/"

check() {
	FILE="$1.s"
	if [ "`diff3 $DIR1/$FILE $DIR2/$FILE $DIR3/$FILE`" != "" ]; then
		echo "$FILE => diff KO"
		exit 1
	fi
	echo "$FILE => OK"
}

check "charutil"
check "fileutil"
check "gen"
check "list"
check "main"
check "parse"
check "parseutil"
check "stack"
check "tokenize"
check "typeutil"
check "varutil"
check "error"

echo "OK!!"
