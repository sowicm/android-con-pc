<?php

clearstatcache();
$hdir = @opendir('./l/');
$files = array();
while ($file = @readdir($hdir))
{
    if ($file == '.' || $file=='..')
        continue;
            
    if (is_file('./l/'.$file))
    {
        $files[] = $file;
    }
}
closedir($hdir);
sort($files);
foreach ($files as $file)
    echo $file."\n";
    
?>
