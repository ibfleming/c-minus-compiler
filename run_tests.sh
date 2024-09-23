#!/bin/bash

# Set the directory and output file
TEST_DIR="./tests"
OUTPUT_FILE="./results.txt"

# Create the output file if it doesn't exist
touch "$OUTPUT_FILE"

# Initialize counters for passed and failed tests
pass_count=0
fail_count=0

# Loop through each .c- file in the test directory
for file in "$TEST_DIR"/*.c-; do
  # Get the filename without the extension
  filename="${file%.c-}"

  # Run the compiler on the .c- file and capture the output
  output=$("./c-" "$file")

  # Get the expected output from the corresponding .out file
  expected_output=$(< "$filename".out)

  # Compare the output with the expected output
  if [ "$output" = "$expected_output" ]; then
    result="PASS"
    ((pass_count++))
  else
    result="FAIL"
    ((fail_count++))
  fi

  # Append the result to the output file
  echo "$filename: $result" >> "$OUTPUT_FILE"

  # If the result is FAIL, append the diff to the output file
  if [ "$result" = "FAIL" ]; then
    echo "Diff for $filename:" >> "$OUTPUT_FILE"
    diff -y -W 192 "$filename".out <(echo "$output") >> "$OUTPUT_FILE"
  fi
done

# Print the summary of test results
total_tests=$((pass_count + fail_count))
OUTPUT_MSG="Total tests passed: $pass_count out of $total_tests"
echo $OUTPUT_MSG >> "$OUTPUT_FILE"