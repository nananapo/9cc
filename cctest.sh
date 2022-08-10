TARGET="$1"
DIR="$2/"

if [ "$TARGET" = "" ]; then
	TARGET="x8664"
fi
if [ "$DIR" = "/" ]; then
	echo "Usage: sh test.sh [x8664/riscv] [dir]"
	exit 1
fi

cd 9cc
echo "`pwd`"

NCC="../9cc/9cc --arch $TARGET "
PRPR="../prpr/prpr --stddir ../std/$TARGET/"

rm -rf $DIR
mkdir $DIR

compile() {
	cname="$1.c"
	aname="$DIR$1.s"

	$PRPR $cname > $DIR$cname
	if [ "$?" != "0" ]; then
		echo "$cname => FAILED TO PRPR"
		exit 1
	fi

	cat $DIR$cname | $NCC >  $aname
	if [ "$?" != "0" ]; then
		echo "$cname => FAILED TO 9cc"
		exit 1
	fi
	echo "$cname compile -> OK"
}

mgcc() {
	cname="$1.c"
	aname="$DIR$1.s"
	gcc -S $cname -o $aname
	echo "$cname => gcc"
}


compile "charutil"
compile "fileutil"
compile "list"
compile "main"
compile "parse"
compile "parseutil"
compile "stack"
compile "tokenize"
compile "typeutil"
compile "varutil"
compile "analyze"
compile "mymath"
compile "il"
compile "abst"
compile "gen_x8664"
compile "gen_riscv"
compile "error"

#mgcc "charutil"
#mgcc "fileutil"
#mgcc "list"
#mgcc "main"
#mgcc "parse"
#mgcc "parseutil"
#mgcc "stack"
#mgcc "tokenize"
#mgcc "typeutil"
#mgcc "varutil"
#mgcc "analyze"
#mgcc "mymath"
#mgcc "il"
#mgcc "abst"
#mgcc "gen_x8664"
#mgcc "get_riscv"
#mgcc "error"

cd $DIR
gcc -o ../9cc charutil.s fileutil.s list.s main.s parse.s parseutil.s stack.s tokenize.s typeutil.s varutil.s analyze.s mymath.s il.s abst.s gen_x8664.s gen_riscv.s error.s
