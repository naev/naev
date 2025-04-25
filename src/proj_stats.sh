#!

# generates three txt files, that can be displayed using show_stats.py

find -P -type f -name "*.h" | while read i;
	do j=${i%h}c;
	if [ -f ${j} ];
		then 
		res=`cat $j | grep -cvx "[ \t]*"`
		echo -e "$res\t${j#./}" 
		echo -n . >&2
	fi
done | sort -nr > modules.txt
echo -e "\rmodules.txt ">&2

find -P -type f -name "*.h" | while read i;
	do if [ -f ${i%h}c ];
		then 
		echo -e "`cat $i | grep -cvx "[ \t]*"`\t${i#./}"
		echo -n . >&2
	fi
done | sort -nr > modules_comments.txt
echo -e "\rmodules_comments.txt ">&2

find -P -type f -name "*.c" | while read i;
	do j=${i%c}h;
	if [ ! -f ${j} ];
		then res=`cat $i | grep -cvx "[ \t]*"`
		echo -e "$res\t${i#./}" 
		echo -n . >&2
	fi
done | sort -nr | sort -nrm modules.txt - > sources.txt
echo -e "\rsources.txt ">&2

find -P -type f -name "*.o" | while read i;
	do size=`wc -c $i | grep -o ^[0-9]*`
	echo -e "${size}\t${i#./}"
	echo -n . >&2
done | sort -nr > objects.txt
echo -e "\robjects.txt ">&2



