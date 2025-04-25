#!

if test -f no_comments; then FLAG=1; fi

make no_comments >/dev/null


c_lines=`find -P -type f -name "*.c" -exec ./no_comments {} \; | grep -cv '^[ \t]*$' `
comments=`find -P -type f -name "*.h" -exec ./no_comments -r {} \; | grep -cv '^[ \t]*$' `
script_lines=`find -P -type f \( -name "*.py" -o -name "*.sh" -o -name "makefile"  \) -exec cat {} \; | grep -cvx '[ \t]*'`

if [[ $FLAG == 1 ]];
	then rm -v no_comments
fi

C1="\e[0;36m"
C2="\e[1;34m"
echo -e "$C1 c code $C2$c_lines$C1  h comments $C2$comments$C1  sh,py,make $C2$script_lines$C1 \e[0m"



