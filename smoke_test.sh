#! bin/bash
# smoke test for assembler
# runs assembler on one input file and compares output to expected output files
# expecting assemlber binary in currend directory
# expecting input file (.as) and output files (.ob .ent .ext) in ./smoke_test directory

# test settings
assembler="iassembler.exe"
asm_filename=$1
EXIT_OK=0

echo "1. cleaning up current directory"
rm $asm_filename.*

echo "2. executing smoke test:"
# copying input file to current directory
cp ./smoke_test/$asm_filename.as .
echo "	running assembler"
./$assembler $asm_filename.as
# check exit code
if [ $? -ne $EXIT_OK ]
then
	exit $?
fi

# compare output in current directory to expected output in ./smoke_test directory 
echo "3. comparing outputs:"
echo "	comparing .ob file:"
diff -a -w -s $asm_filename.ob ./smoke_test/$asm_filename.ob
echo "	comparing .ent file:"
diff -a -w -s $asm_filename.ent ./smoke_test/$asm_filename.ent
echo "	comparing .ext file:"
diff -a -w -s $asm_filename.ext ./smoke_test/$asm_filename.ext

exit 0
