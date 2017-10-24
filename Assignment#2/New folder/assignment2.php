<?php

# Getting POST data sent from the client
$red = $_POST["RED"];
$ir = $_POST["IR"];

# Create a string to write to the file
$temp = $red.", ".$ir."\n";

# Path of file
$file = '/tmp/data3.csv';

# Open a file and write the data into it
if($handle = fopen($file, 'a')) {
    fwrite($handle, $temp);
    fclose($handle);
} else {
    echo "FAILED: Could not open file for writing.";
}
?>
