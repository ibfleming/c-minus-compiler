#!/bin/bash

# Set the directory and output file
HW_DIR="hw3"
TEST_DIR="./tests/${HW_DIR}"
OUTPUT_FILE="./results.txt"

# Create the output file if it doesn't exist
touch "$OUTPUT_FILE"

# Initialize counters for passed and failed tests
pass_count=0
fail_count=0

# Define the list of filenames that require the -P argument
declare -a special_files=("while.c-" "uninit2.c-" "mixedControl.c-" "mixedControl3.c-" "mixedControl2.c-" "init.c-" "init4.c-" "ifNest.c-" "if.c-" "forb.c-" "fora.c-" "expR.c-" "allgood.c-" "break3.c-" "break4.c-" "break.c-" "decl.c-" "emptyline.c-" "everythingF24.c-" "expL.c-") # Add your filenames here

# Loop through each .c- file in the test directory
for file in "$TEST_DIR"/*.c-; do
  # Get the filename without the extension
  filename="${file%.c-}"

  args=""

  # Check if the current file matches any in the special_files list
  if [ "$HW_DIR" = "hw3" ]; then # Corrected syntax here
    for special_file in "${special_files[@]}"; do
      if [[ "$(basename "$file")" == "$special_file" ]]; then
        args="-P" # Add the -P argument
        break     # No need to check further if a match is found
      fi
    done
  fi

  # Run the compiler on the .c- file and capture the output
  output=$("./c-" $args "$file")

  # Get the expected output from the corresponding .out file
  expected_output=$(<"$filename".out)

  # Compare the output with the expected output
  if [ "$output" = "$expected_output" ]; then
    result="PASS"
    ((pass_count++))
  else
    result="FAIL"
    ((fail_count++))
  fi

  # Append the result to the output file
  echo "$filename: $result" >>"$OUTPUT_FILE"

  # If the result is FAIL, append the diff to the output file
  if [ "$result" = "FAIL" ]; then
    echo "Diff for $filename:" >>"$OUTPUT_FILE"
    echo "EXPECTED | MINE" >>"$OUTPUT_FILE"
    diff -y -W 192 "$filename".out <(echo "$output") >>"$OUTPUT_FILE"
  fi
done

# Print the summary of test results
total_tests=$((pass_count + fail_count))
OUTPUT_MSG="Total tests passed: $pass_count out of $total_tests"
echo "$OUTPUT_MSG" >>"$OUTPUT_FILE"
