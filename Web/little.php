<?php
$key = 'KDIWXCBKPACVNMZ';
if ($_GET['key'] == $key)
{
    foreach ($_GET['targets'] as $target)
    {
        $len = strlen($target);
        $flag = true;
        for ($i = 0; $i < $len; ++$i)
        {
            if (! ($target[$i] >= 'a' && $target[$i] <= 'z' || $target[$i] >= 'A' && $target[$i] <= 'Z' || $target[$i] >= '0' && $target[$i] <= '9') )
            {
                $flag = false;
                break;
            }
        }
        if ($flag)
        {
            file_put_contents('c/'.$target, iconv('UTF-8', 'GB2312', 'SFLBU['.time().'#'.htmlspecialchars($_GET['q'], ENT_QUOTES).']'));
        }
    }
}
unset($_GET);
?>