NAME="$1"

DIR1="$NAME/test1/"
DIR2="$NAME/test2/"
DIR3="$NAME/test3/"

check() {
	FILE="$1.s"
	if [ "`diff $DIR1/$FILE $DIR2/$FILE`" != "" ]; then
		echo "$FILE => diff 1 != 2 KO"
		exit 1
	fi
	if [ "`diff $DIR2/$FILE $DIR3/$FILE`" != "" ]; then
		echo "$FILE => diff 2 != 3 KO"
		exit 1
	fi
	echo "$FILE => OK"
}

find $DIR1 -name "*.s" | awk -F/ "{print \$NF}" | awk -F. "{print \$1}" | while read line
do
	check "$line"
done
