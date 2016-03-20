<?php
$s = $_SERVER["REMOTE_ADDR"];
$s = str_replace('.', 'x', $s);
file_put_contents('l/'.$s, '');
echo '['.$s.']';
?>