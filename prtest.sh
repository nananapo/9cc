DIR="$1/"

cd prpr
echo "`pwd`"

NCC="../9cc/9cc"
PRPR="./prpr --stddir ../std/"

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


compile "fileutil"
compile "gen"
compile "main"
compile "parse"
compile "strutil"
compile "tokenize"
compile "tokenutil"
compile "error"

#mgcc "fileutil"
#mgcc "gen"
#mgcc "main"
#mgcc "parse"
#mgcc "strutil"
#mgcc "tokenize"
#mgcc "tokenutil"
#mgcc "error"

cd $DIR
gcc -o ../prpr fileutil.s gen.s main.s parse.s strutil.s tokenize.s tokenutil.s error.s
