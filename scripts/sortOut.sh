#!/bin/bash

# Check if a .c file is provided as input
if [ "$#" -lt 1 ] || [[ ! "$1" == *.c- ]]; then
    echo "Usage: ./sort_output.sh <file> [-c]"
    echo "Note: <file> must be a .c- file and -c is optional to print output to console"
    exit 1
fi

# Compile the C file and capture output in out.txt
./c- "$1" > out.txt

# Extract and store ERROR(LINKER) lines temporarily
linker_errors=$(grep "ERROR(LINKER)" out.txt)

# Sort the rest of the lines, excluding ERROR(LINKER) and Number of warnings/errors lines
grep -v "ERROR(LINKER)" out.txt | \
grep -v -E "Number of (warnings|errors)" | \
awk '{print ($0 ~ /^[A-Za-z]/ ? $0 : "A" $0)}' | \
sort -t '(' -k2,2n -k1,1 -k3,3 | \
awk '{sub(/^A/, ""); print}' > out_sorted.txt

# Append the ERROR(LINKER) lines back to the sorted output
echo "$linker_errors" >> out_sorted.txt

# Append the Number of warnings and Number of errors at the very end
grep "Number of warnings" out.txt >> out_sorted.txt
grep "Number of errors" out.txt >> out_sorted.txt

# Overwrite out.txt with the final sorted output
mv out_sorted.txt out.txt

# Check if the -c flag is provided to print the result to console
if [[ "$2" == "-c" ]]; then
    cat out.txt
fi

echo "Sorting complete. Output saved to out.txt."
