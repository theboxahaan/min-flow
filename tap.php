<?php
    $curl_resource = curl_init();
    $opt = array(
                CURLOPT_URL => 'http://localhost:8080' ,
                CURLOPT_POST => true,
                CURLOPT_POSTFIELDS => [ 'I1' => 'fuck', 'I2' => 'off']
                );

    curl_setopt_array($curl_resource, $opt);
    $resp = curl_exec($curl_resource);
    var_dump($resp);

    curl_close($curl_resource);
?>