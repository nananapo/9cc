DIR1="prpr/test1/"
DIR2="prpr/test2/"
DIR3="prpr/test3/"

check() {
	FILE="$1.s"
	if [ "`diff3 $DIR1/$FILE $DIR2/$FILE $DIR3/$FILE`" != "" ]; then
		echo "$FILE => diff KO"
		exit 1
	fi
	echo "$FILE => OK"
}

check "fileutil"
check "gen"
check "main"
check "parse"
check "strutil"
check "tokenize"
check "tokenutil"
check "error"

echo "OK!!"
