#!/bin/bash

mkdir -p target
mkdir -p target/C target/Java target/Python
touch result.csv

if [ "$6" != "-noexecute" ];then
    echo "student_id,type,matched,not_matched" >> result.csv
fi

path=""
extension=""
test_Number=$(ls "$3" | wc -l)


dfsVisit()
{
    if [ -d "$1" ]
	then
	
		for i in "$1"/*
		do
			dfsVisit "$i"
		done
	
	elif [ -f "$1" ]
	then
		temp_s="$1"
        _extension=${temp_s#*.}
        if [ "$_extension" = 'c' ];then
            path="$1"
            extension="$_extension"
            return 0
        elif [ "$_extension" = 'java' ]
        then
            path="$1"
            extension="$_extension"
            return 0
        elif [ "$_extension" = 'py' ]
        then
            path="$1"
            extension="$_extension"
            return 0
        fi

	fi
}

visit()
{
    if [ "$5" = "-v" ];then
        echo "Found $test_Number test files"
    fi
    mkdir -p temp_dir
    for i in "$1"/*
    do
        t="$i"
        roll=${t: -11:7}
        if [ "$5" = "-v" ];then
            echo "Organizing files of $roll"
        fi
        unzip -qq -d temp_dir "$i"
        

        dfsVisit temp_dir
        
        if [ "$5" = "-v" ];then
            echo "Executing files of $roll"
        fi
        if [ "$extension" = "c" ];then
            mkdir -p target/C/"$roll"
            cp "$path" target/C/"$roll"/main.c
            if [ "$6" != "-noexecute" ];then
                gcc target/C/"$roll"/main.c -o target/C/"$roll"/main.out
                
                #echo $test_Number
                matched=0
                unmatched=0
                for((j=1;j<=$test_Number;j++))
                do
                    ./target/C/"$roll"/main.out < "$3"/test$j.txt > target/C/"$roll"/out$j.txt
                    difference=$(diff target/C/"$roll"/out$j.txt "$4"/ans$j.txt | wc -l)
                    # echo $difference

                    if [ $difference -gt 0 ];then
                        unmatched=$(expr $unmatched + 1)
                    else 
                        matched=$((matched + 1))
                    fi
                    
                done

                echo "$roll,$extension,$matched,$unmatched" >> result.csv


                
                # write to csv file

            fi

        elif [ "$extension" = "java" ];then
            mkdir -p target/Java/"$roll"
            cp "$path" target/Java/"$roll"/Main.java
            if [ "$6" != "-noexecute" ];then
                javac target/Java/"$roll"/Main.java
                #This will create a .class file at the same directory as the .java file
                matched=0
                unmatched=0
                for((j=1;j<=$test_Number;j++))
                do
                    java -cp target/Java/"$roll" Main < "$3"/test$j.txt > target/Java/"$roll"/out$j.txt
                    difference=$(diff target/Java/"$roll"/out$j.txt "$4"/ans$j.txt | wc -l)

                    if [ $difference -gt 0 ];then
                        unmatched=$(expr $unmatched + 1)
                    else 
                        matched=$((matched + 1))
                    fi

                done

                echo "$roll,$extension,$matched,$unmatched" >> result.csv

                #write to csv file
                
            fi

        elif [ "$extension" = "py" ];then
            mkdir -p target/Python/"$roll"
            cp "$path" target/Python/"$roll"/main.py
            if [ "$6" != "-noexecute" ];then

                matched=0
                unmatched=0
                for((j=1;j<=$test_Number;j++))
                do
                    python3 target/Python/"$roll"/main.py < "$3"/test$j.txt > target/Python/"$roll"/out$j.txt
                    difference=$(diff target/Python/"$roll"/out$j.txt "$4"/ans$j.txt | wc -l)

                    if [ $difference -gt 0 ];then
                        unmatched=$(expr $unmatched + 1)
                    else 
                        matched=$((matched + 1))
                    fi

                done

                echo "$roll,$extension,$matched,$unmatched" >> result.csv

                #write to csv file
                
            fi
        
        fi


        rm -fr temp_dir/*
    done
    rm -rf temp_dir
	
}

visit $*
