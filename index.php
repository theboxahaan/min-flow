<?php
    $filename = './post-log.txt';
    file_put_contents($filename, $_POST, FILE_APPEND);
?>