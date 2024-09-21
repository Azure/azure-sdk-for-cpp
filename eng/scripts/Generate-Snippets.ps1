# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Powershell script to generate snippets from C++ source files.
# Usage: generate_snippets.ps1 <source_dir> <output_dir>

param(
	[Parameter(Mandatory = $true)]
	[string]$source_dir,

	[Parameter(Mandatory = $true)]
	[string]$output_dir,

	[Parameter(Mandatory = $false)][switch]
	[bool]$verify = $false
)

function ParseSnippets {
	param (
		[Parameter(Mandatory = $true)]
		[string[]]$input_files
	)

	# for each file in $input_files, read the contents of the file into a string, then search the string for patterns delimited by @begin_snippet and @end_snippet.
	# for each pattern found, extract the snippet name and contents, and save the snippet in a map of snippet name to snippet contents.
	# then, for each snippet in the map, write the snippet contents to a file in $output_dir with the snippet name as the file name.
	#$snippet_pattern = '@begin_snippet:\s+(?<snippet_name>\w+)\s+(?<snippet_contents>.*?)@end_snippet'

	$snippet_map = @{}
	$snippet_pattern = '@begin_snippet:\s+(?<snippet_name>\w+)\s+\n(?<snippet_contents>.*?)\s+(//|`*/)\s+@end_snippet'
	foreach ($cpp_file in $input_files) {
		Write-Host "Scanning source: $cpp_file"
		$cpp_file_contents = Get-Content $cpp_file -Raw
		$snippet_matches = [regex]::Matches($cpp_file_contents, $snippet_pattern, 'Singleline')

		foreach ($snippet_match in $snippet_matches) {
			Write-Host "Found snippet: $($snippet_match.Groups['snippet_name'].Value)"
			$snippet_name = $snippet_match.Groups['snippet_name'].Value
			$snippet_contents = $snippet_match.Groups['snippet_contents'].Value
			if ($snippet_map[$snippet_name]) {
				Write-Host "ERROR: Duplicate snippet name: $snippet_name"
				exit 1
			}
			else {
				$snippet_map[$snippet_name] = $snippet_contents
			}
		}
	}
	return (, $snippet_map)
}

function ProcessSnippetsInFile {
	param (
		[Parameter(Mandatory = $true)]
		[hashtable]$snippet_map,
		[Parameter(Mandatory = $true)]
		[object]$output_file
	)
	$output_file_contents = Get-Content $output_file -Raw
	$snippet_matches = [regex]::Matches($output_file_contents, '@insert_snippet:\s+(?<snippet_name>\w+)', 'Singleline')

	# if there is no match, we don't need to do anything else.
	if ($snippet_matches.Count -eq 0) {
		return $true;
	}

	$original_file_contents = $output_file_contents

	foreach ($snippet_match in $snippet_matches) {
		$snippet_name = $snippet_match.Groups['snippet_name'].Value
		Write-Host "Replacing snippet $snippet_name in file $output_file."
		if (!$snippet_map[$snippet_name]) {
			Write-Host "ERROR: Unknown snippet name: $snippet_name in file $output_file"
			return $false
		}


		if ($output_file.Extension -eq '.md') {

			# Remove the existing snippet text, if any.
			$output_file_contents = [Regex]::Replace($output_file_contents, "<!--\s+@insert_snippet:\s+$snippet_name\s*-->\s+``````cpp.+?``````\s+", "<!-- @insert_snippet: $snippet_name -->`r`n`r`n", 'Singleline')

			# Insert the snippet text.
			$snippet_text = $snippet_map[$snippet_name]
			
			# Remove leading spaces from each line, by first splitting the text into lines.
			$lines = $snippet_text -split [Environment]::NewLine

			# Then, find the minimum leading whitespace across all lines.
			# This is done to trim the minimum amount of leading whitespace from each line while preserving the relative indentation for lines that are already indented.
			# When calculating the min whitespace, we only consider lines that are not empty.
			$minWhitespace = ($lines | Where-Object { $_.Trim() -ne '' } | ForEach-Object { $_.IndexOf($_.TrimStart()) }) | Measure-Object -Minimum | Select-Object -ExpandProperty Minimum

			# Trim the minimum whitespace from each line
			$trimmedLines = $lines | ForEach-Object { 
				if ($_.Length -gt $minWhitespace) {
					$_.Substring($minWhitespace)
				} else {
					$_
				}
			}

			# Join the lines back into a single string
			$snippet_text_clean = $trimmedLines -join [Environment]::NewLine

			$output_file_contents = $output_file_contents -replace "<!--\s+@insert_snippet:\s+$snippet_name\s*-->\s+", "<!-- @insert_snippet: $snippet_name -->`r`n``````cpp`r`n$snippet_text_clean`r`n```````r`n`r`n"

		}
		elseif ($output_file.Extension -eq '.hpp') {
			$output_file_contents = $output_file_contents -replace '@insert_snippet:\s+(?<snippet_name>\w+)', '$snippet_map[$snippet_name]'
		}
		elseif ($output_file.Extension -eq '.cpp') {
			$output_file_contents = $output_file_contents -replace '@insert_snippet:\s+(?<snippet_name>\w+)', '$snippet_map[$snippet_name]'
		}
		else {
			Write-Host "ERROR: Unknown file extension: $output_file"
			return $false
		}

	}
	# The Regex::Replace above inserts an extra newline at the end of the file. Remove it.
	$output_file_contents = $output_file_contents -replace "`r`n\s*\Z", ""
	$original_contents = $original_file_contents -replace "`r`n\s*\Z", ""

	if ($verify) {
		if ($output_file_contents -ne $original_contents) {
			Write-Host "ERROR: Snippet contents does not match for file: $output_file."
			return $false
		}
	}
 elseif (!$verify) {
		Write-Host "Writing file: $output_file"
		Set-Content -Path $output_file.FullName -Value $output_file_contents
	}
	return $true

}

$source_dir = Resolve-Path $source_dir
$output_dir = Resolve-Path $output_dir

$input_files = Get-ChildItem -Path $source_dir -Include *.cpp, *.hpp -Recurse

# The snippet generator only processes markdown files currently.
$output_files = Get-ChildItem -Path $output_dir -Include *.md -Recurse

$snippet_map = @{}
$snippet_map = ParseSnippets($input_files)

# for each file in $output_files, read the contents of the file, searching for a string @insert_snippet: <snippet_name>. Insert the corresponding snippet from the $snippet_map
# into the file at that location and write it out.
$failed = $false
foreach ($output_file in $output_files) {
	$result = ProcessSnippetsInFile -snippet_map $snippet_map -output_file $output_file
	if (!$result) {
		$failed = $true
	}
}
if ($failed) {
	Write-Host "ERROR: Snippet generation failed."

	Write-Host "`r`nTo fix this error, run the following command locally:"
	Write-Host "`r`n`r`n`t powershell -ExecutionPolicy Bypass -File eng/scripts/Generate-Snippets.ps1 -source_dir $source_dir -output_dir $output_dir`r`n"
	Write-Host "`r`nThen, run the following command to verify the changes."
	Write-Host "`r`n`r`n`t powershell -ExecutionPolicy Bypass -File eng/scripts/Generate-Snippets.ps1 -source_dir $source_dir -output_dir $output_dir -verify`r`n"
	Write-Host "`r`nFinally, commit the changes and push to the remote branch."

	exit 1
}
