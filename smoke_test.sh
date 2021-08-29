#! bin/bash
# smoke test for assembler
# script arguments: list of .as files to check without the .as suffix
# runs assembler on inputs files and compares output to expected output files
# expecting assemlber binary in currend directory
# expecting input files (.as) and output files (.ob .ent .ext) in ./smoke_test directory

# test settings
assembler="assembler.exe"
asm_args=""
EXIT_OK=0

echo "1. PRETEST ... preparing working directory"
# deleting older files if any
for arg in "$@"
do
	rm $arg.*
done
# copying input file to current directory
for arg in "$@"
do
	cp ./smoke_test/$arg.as .
done

echo "2. TEST ... running assembler"
# build args for assembler - all script args + .as suffix
for arg in "$@"
do
	asm_args+="$arg.as "
done
echo "	executing ./$assembler $asm_args"
./$assembler $asm_args
# check exit code
if [ $? -ne $EXIT_OK ]
then
	exit $?
fi
 
echo "3. TRIAGE ... comparing outputs:"
# compare output in current directory to expected output in ./smoke_test directory 
for arg in "$@"
do
	echo "=========================================================="
	echo "	comparing $arg.ob:"
	diff -a -w -s $arg.ob ./smoke_test/$arg.ob
	echo "	comparing $arg.ent:"
	diff -a -w -s $arg.ent ./smoke_test/$arg.ent
	echo "	comparing $arg.ext:"
	diff -a -w -s $arg.ext ./smoke_test/$arg.ext
	echo "=========================================================="
done

exit $EXIT_OK
