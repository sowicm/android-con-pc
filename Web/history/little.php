<?php
session_start();
$pwd = 'ohohohoh';

if ($_SESSION['pwd'] == $pwd)
{
    $_SESSION['p'] = $_POST['act'];
}
else if ($_POST['pwd'] == $pwd)
    $_SESSION['pwd'] = $pwd;

unset($_POST);

function scmd($cid)
{
    file_put_contents('c.html', 'SFLBU['.$cid.'#'.time().']');
}

?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"  />
<title>Little</title>
</head>

<body>
<?php
if ($_SESSION['pwd'] != $pwd)
{ ?>
<form method="post">
<input type="password" name="pwd" />
<input type="submit" value="Login" />
</form>
<?php 
}
else
{
  switch ($_SESSION['p'])
  {
  case 'showgif':
      scmd('1');
      break;
  case 'lbutton':
      scmd('2');
      break;
  case 'rbutton':
      scmd('3');
      break;
  case 'desktop':
      scmd('4');
      break;
  case 'up':
      scmd('5');
      break;
  case 'down':
      scmd('6');
      break;
  case 'dark':
      scmd('7');
      break;
  case 'clear':
      scmd('0');
      break;
  }
  unset($_SESSION['p']);
 ?>
<script type="text/javascript">
var actsname = new Array('显示图片', '鼠标左键', '鼠标右键', '更改桌面', '向上', '向下', '黑屏', '清除指令');
var acts     = new Array('showgif',  'lbutton',  'rbutton',  'desktop',  'up',   'down', 'dark', 'clear');

var i;
for (i in acts)
{
    document.write('<form method="post"><input type="hidden" name="act" value="' + acts[i] + '" /><input type="submit" value="' + actsname[i] + '" /></form>');
}
</script>
<?php
}
?>
</form>
</body>
</html>