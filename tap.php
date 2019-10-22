<?php
    $curl_resource = curl_init();
    $opt = array(
                CURLOPT_URL => 'https://min-flow.herokuapp.com' ,
                CURLOPT_POST => true,
                CURLOPT_POSTFIELDS => [ 'I1' => 'glib', 'I2' => 'off']
                );

    curl_setopt_array($curl_resource, $opt);
    $resp = curl_exec($curl_resource);
    var_dump($resp);

    curl_close($curl_resource);
?>