<?php

# Getting POST data sent from the client
$data = $_POST["data"];

# Path of file
$file = '/tmp/assignment4.csv';

# Open a file and write the data into it
if($handle = fopen($file, 'a')) {
    fwrite($handle, $data);
    fclose($handle);
} else {
    echo "FAILED: Could not open file for writing.";
}
?>