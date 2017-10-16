<html>
    <head>
        <title>Group 4 Project 1</title>
    </head>

    <body>
    Test page receives POST data from ESP8266
    <?php

    $date = new DateTime();
    $date = $date->format("d-M-Y  h:i:s  ");

    if(!empty($_POST["key"]))   {
        $tempesp = ($_POST["key"]);
        file_put_contents("/tmp/Temp.out", $date);
        file_put_contents("/tmp/Temp.out", $tempesp, FILE_APPEND);
    }

    ?>
    </body>
</html>