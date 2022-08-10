TARGET="$1"
if [ "$TARGET" = "" ]; then
	TARGET="x8664"
fi

testdir="../test/"
ncc="./9cc --arch $TARGET "
prpr="../prpr/prpr --stddir ../std/$TARGET/"
module="$testdir/sub/print.c"

echo target : $TARGET
echo testdir: $testdir
echo 9cc    : $ncc
echo prpr   : $prpr

rm -rf tmp
mkdir tmp
touch tmp/err

echo test start
sleep 1


prefix="
int pint(int i);
int dint(int i);
int pchar(char a);
int pspace(int n);
int pline();
int my_putstr(char *s, int n);
int my_print(char *s);
int *my_malloc_int(int n);
"

COUNTER=0

assert_async(){
	uni="$1"

	asmname="./tmp/asm_$uni.s"
	cname="./tmp/c_$uni.c"
	targetname="./tmp/exe_$uni"
	actresname="./tmp/act_$uni.txt"
	expresname="./tmp/exp_$uni.txt"

	echo "$prefix" > $cname
	cat "$testdir/$1" >> $cname

    $prpr "$cname" | $ncc > $asmname
	if [ "$?" != "0" ]; then
		echo "9CC KO => $1"
		echo "9CC KO => $1" >> tmp/err
		exit 1
	fi
	
	cc -o $targetname $asmname $module
	if [ "$?" != "0" ]; then
		echo "$input" > hey
		echo "9CC COMPILE KO => $1"
		echo "9CC COMPILE KO => $1" >> tmp/err
		exit 1
	fi

	./$targetname &> $actresname
	actual_status="${PIPESTATUS[0]}"

	cc -w -o $targetname $cname $module
	if [ "$?" != "0" ]; then
		echo "GCC FAIL => $1"
		echo "GCC FAIL => $1" >> tmp/err
		exit 1
	fi

	./$targetname &> $expresname
	expected_status="${PIPESTATUS[0]}"

	if [ "$actual_status" != "$expected_status" ]; then
  	  echo "STATUS KO $1 > act:$actual_status exp:$expected_status"
  	  echo "STATUS KO $1 > act:$actual_status exp:$expected_status" >> tmp/err
  	  exit 1
	fi

  	if [ "`diff $actresname $expresname`" != "" ]; then
  	  echo "OUTPUT KO $1 > $actresname $expresname"
  	  echo "OUTPUT KO $1 > $actresname $expresname" >> tmp/err
  	  exit 1
  	fi
}

assert() {
	COUNTER=$((++COUNTER))
	assert_async "$1" "$1"_"$COUNTER" &
}

start_time=`date +%s`

assert "expr1.c"
assert "expr2.c"

assert "opadd.c"
assert "opmin1.c"
assert "opmin2.c"
assert "opmin3.c"
assert "opmul1.c"
assert "opmul2.c"
assert "opdiv.c"
assert "par.c"

assert "unaryadd1.c"
assert "unarymin1.c"
assert "unarymin2.c"
assert "unarymin3.c"
assert "unarymin4.c"

assert "opequal1.c"
assert "opequal2.c"
assert "opnequal1.c"
assert "opnequal2.c"
assert "opgreat1.c"
assert "opgreat2.c"
assert "opgreat3.c"

assert "defvar1.c"
assert "defvar2.c"
assert "defvar3.c"
assert "defvar4.c"
assert "defvar5.c"
assert "defvar6.c"
assert "defvar7.c"
assert "defvar8.c"
assert "defvar9.c"
assert "defvar10.c"
assert "defvar11.c"
assert "defvar12.c"
assert "defvar13.c"
assert "defvar14.c"
assert "defvar15.c"
assert "defvar16.c"

assert "return1.c"
assert "return2.c"
assert "return3.c"

assert "ifret1.c"
assert "ifret2.c"
assert "ifret3.c"
assert "ifret4.c"
assert "ifret5.c"

assert "while1.c"
assert "while2.c"

assert "for1.c"
assert "for2.c"
assert "fornone.c"
assert "for3.c"
assert "for4.c"
assert "for5.c"
assert "for6.c"
assert "for7.c"

assert "stmtexpr1.c"
assert "stmtexpr2.c"

assert "funccall1.c"
assert "funccall2.c"
assert "funccall3.c"
assert "funccall4.c"
assert "funccall5.c"
assert "add6.c"
assert "fib.c"

assert "addr1.c"
assert "addr2.c"
assert "addr3.c"
assert "addr4.c"
assert "addr5.c"
assert "addr6.c"
assert "addr7.c"
assert "addr8.c"
assert "addr9.c"

assert "sizeof1.c"
assert "sizeof2.c"
assert "sizeof3.c"
assert "sizeof4.c"
assert "sizeof5.c"
assert "sizeof6.c"
assert "sizeof7.c"
assert "sizeof8.c"
assert "sizeof9.c"
assert "sizeof10.c"
assert "sizeof11.c"
assert "sizeof12.c"
assert "sizeof13.c"
assert "sizeof14.c"
assert "sizeof15.c"
assert "sizeof16.c"
assert "sizeof17.c"
assert "sizeof18.c"
assert "sizeof19.c"
assert "sizeof20.c"
assert "sizeof21.c"

assert "array1.c"
assert "array2.c"
assert "array3.c"
assert "array4.c"
assert "array5.c"
assert "array6.c"
assert "array7.c"
assert "array8.c"
assert "array9.c"
assert "array10.c"
assert "array11.c"
assert "array12.c"
assert "array13.c"
assert "array14.c"
assert "array15.c"
assert "array16.c"
assert "array17.c"
assert "array18.c"
assert "array19.c"
assert "array20.c"
assert "array21.c"
assert "array22.c"
assert "array23.c"
assert "array24.c"
assert "array25.c"
assert "array26.c"
assert "array27.c"
assert "array28.c"
assert "array29.c"
assert "array30.c"
assert "array31.c"
assert "array32.c"
assert "array33.c"
assert "array34.c"
assert "array35.c"
assert "array36.c"
assert "array37.c"

assert "global1.c"
assert "global2.c"
assert "global3.c"
assert "global4.c"
assert "global5.c"
assert "global6.c"
assert "global7.c"
assert "global8.c"
assert "global9.c"
assert "global10.c"
assert "global11.c"
assert "global12.c"
assert "global13.c"
assert "global14.c"
assert "global15.c"
assert "global16.c"
assert "global17.c"
assert "global18.c"
assert "global19.c"
assert "global20.c"
assert "global21.c"
assert "global22.c"
assert "global23.c"
assert "global24.c"
assert "global25.c"
assert "global26.c"
assert "global27.c"
assert "global28.c"
assert "global29.c"
assert "global30.c"
assert "global31.c"
assert "global32.c"
assert "global33.c"
assert "global34.c"
assert "global35.c"

assert "charptr1.c"
assert "charptr2.c"
assert "charptr3.c"
assert "charptr4.c"
assert "charptr5.c"

assert "str1.c"
assert "str2.c"
assert "str3.c"
assert "str4.c"
assert "str5.c"
assert "str6.c"
assert "str7.c"

#assert "8queen.c"
assert "9queen.c"

assert "structsize1.c"
assert "structsize2.c"
assert "structsize3.c"
assert "structsize4.c"
assert "structsize5.c"
assert "structsize6.c"

assert "struct1.c"
assert "struct2.c"
assert "struct3.c"
assert "struct4.c"
assert "struct5.c"
assert "struct6.c"
assert "struct7.c"
assert "struct8.c"
assert "struct9.c"
assert "struct10.c"
assert "struct11.c"
assert "struct12.c"
assert "struct13.c"
assert "struct14.c"
assert "struct15.c"

assert "arg1.c"
assert "arg2.c"
assert "arg3.c"
assert "arg4.c"
assert "arg5.c"
assert "arg6.c"
assert "arg7.c"
assert "arg8.c"
assert "arg9.c"
assert "arg10.c"
assert "arg11.c"

assert "varg.c"
assert "varg2.c"
assert "varg3.c"

assert "void1.c"
assert "void2.c"

assert "cast0.c"
assert "cast1.c"
assert "cast2.c"
assert "cast3.c"
assert "cast4.c"

assert "implicit_cast0.c"

assert "cond1.c"

assert "ctype.c"

assert "increments1.c"
assert "increments2.c"
assert "increments3.c"
assert "increments4.c"
assert "increments5.c"
assert "increments6.c"
assert "increments7.c"
assert "increments8.c"
assert "increments9.c"
assert "increments10.c"
assert "increments11.c"
assert "increments12.c"
assert "increments13.c"
assert "increments14.c"

assert "not.c"

assert "k_r_strcpy.c"

assert "dowhile.c"
assert "return.c"

assert "continue1.c"
assert "continue2.c"
assert "continue3.c"

assert "break1.c"
assert "break2.c"
assert "break3.c"
assert "break4.c"

assert "nostmt1.c"

assert "compound_assignment1.c"

assert "mod.c"

assert "declare_assign.c"

assert "switch1.c"
assert "switch2.c"
assert "switch3.c"

assert "duffsdevice.c"

assert "typedef1.c"
assert "defstruct1.c"
assert "defstruct2.c"

assert "elseif.c"

assert "enum1.c"
assert "enum2.c"
assert "enum3.c"
assert "enum4.c"
assert "enum5.c"
assert "enum6.c"

assert "stack.c"

assert "union1.c"
assert "union2.c"
assert "union3.c"
assert "union4.c"

assert "condop.c"
assert "condop2.c"

assert "bitwise_and.c"
assert "bitwise_or.c"
assert "bitwise_not.c"
assert "bitwise_xor.c"

assert "shift.c"

assert "ret_struct0.c"
assert "ret_struct1.c"
assert "ret_struct2.c"
assert "ret_struct3.c"
assert "ret_struct4.c"
assert "ret_struct5.c"
assert "ret_struct6.c"
assert "ret_struct7.c"

assert "quine.c"

wait

end_time=`date +%s`

echo time : $((end_time - start_time)) sec

if [ "`cat tmp/err | wc -l | tr -d ¥" ¥"`" != "0" ]; then
	echo "--- report ---------------"
	cat tmp/err | sort
	echo "test failed : `wc -l tmp/err | tr -d " tmp/er"` errors"
	exit 1
fi

echo "test OK!"
