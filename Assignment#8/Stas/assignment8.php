<?php
# Getting POST data sent from the client
$accelerometer_data = $_POST["lis3dh"];
$particle_sensor_data = $_POST["max30105"];
# Path of file
$file1 = '/tmp/assignment6_max30105New.csv';
$file2 = '/tmp/assignment6_lis3dhNew.csv';
# Open a file and write the data into it
if($handle = fopen($file1, 'a')) {
    fwrite($handle, $particle_sensor_data);
    fclose($handle);
} else {
    echo "FAILED: Could not open file1 for writing.";
}
if($handle = fopen($file2, 'a')) {
    fwrite($handle, $accelerometer_data);
    fclose($handle);
} else {
    echo "FAILED: Could not open file2 for writing.";
}
?>