<?php
define("PORT", 42069);
error_reporting(-1);
session_start();
$perfect=array("HELO" => "250","DATA" => "354","SEND" => "250","QUIT" => "251");

$fp = fsockopen("localhost", constant("PORT"), $errno, $errstr);
$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
socket_connect($socket, '127.0.0.1', 42069);
function transaction($l){
    echo "HTTP SERVER: ".$l."<br>";
    socket_send($GLOBALS['socket'], $l, strlen($l), 0);
    socket_recv($GLOBALS['socket'], $lol, 3,0);
    echo "SMTP SERVER: ".$lol."<br><br>";
    //NOT THE SUCCESS RESPONSE CODE
    if(($lol!="250") and ($lol!=$GLOBALS['perfect'][$l])){
        socket_close($GLOBALS['socket']);
        echo "Disconnecting...";
        die();
    }
}
function transaction_d($l){
    socket_send($GLOBALS['socket'], $l, strlen($l), 0);
}
transaction("HELO");



//special case for escaping <email> in HTML
echo "HTTP SERVER: MAIL FROM:&lt;".$_POST['f']."&gt;"."<br>";
socket_send($GLOBALS['socket'], "MAIL FROM:<".$_POST['f'].">", strlen("MAIL FROM:<".$_POST['f'].">"), 0);
socket_recv($GLOBALS['socket'], $lol, 3,0);
echo "SMTP SERVER: ".$lol."<br><br>";
if($lol!="250"){
    socket_close($GLOBALS['socket']);
    echo "Disconnecting...";
    die();
}



//special case for multiple recipients and escaping <email> in HTML
$pieces = explode(" ", $_POST['t']);
foreach ($pieces as $val) {
    echo "HTTP SERVER: MAIL FROM:&lt;".$val."&gt;"."<br>";
socket_send($GLOBALS['socket'], "RCPT TO:<".$val.">", strlen("MAIL FROM:<".$val.">"), 0);
socket_recv($GLOBALS['socket'], $lol, 3,0);
    echo "SMTP SERVER: " . $lol . "<br><br>";
    if($lol!="250"){
        socket_close($GLOBALS['socket']);
        echo "Disconnecting...";
        die();
    }
}


transaction("DATA");
transaction_d($_POST['text']);
transaction(".\r\n");
transaction("SEND");
transaction("QUIT");
?>