#!/bin/bash

# Check if output file name is provided
if [ $# -lt 1 ]; then
    echo "Usage: $0 output_file [input_files...]"
    exit 1
fi

# Store output file name (first argument) and get its full path
output_file=$1
output_full_path=$(realpath "$output_file")
shift # Remove first argument, leaving input files

# Check if any input files are provided
if [ $# -eq 0 ]; then
    echo "Error: No input files specified"
    exit 1
fi

# Clear output file if it exists
> "$output_file"

# Write output file full path
echo "Output file: $output_full_path" >> "$output_file"
echo "" >> "$output_file"

# Concatenate all input files to output file with formatting
for file in "$@"; do
    if [ -f "$file" ]; then
        # Get full path of input file
        input_full_path=$(realpath "$file")
        # Append input filename with full path
        echo "Input file: $input_full_path" >> "$output_file"
        # Append empty line
        echo "" >> "$output_file"
        # Append file contents
        cat "$file" >> "$output_file"
        # Append separator (36 equal signs)
        echo "=====================================" >> "$output_file"
    else
        echo "Warning: File '$file' does not exist"
    fi
done

echo "Concatenation complete. Output saved to $output_full_path"
