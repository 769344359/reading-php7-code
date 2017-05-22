--TEST--
mb_ereg_replace_callback()
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip mbstring not available');
function_exists('mb_ereg_replace_callback') or die("skip mb_ereg_replace_callback() is not available in this build");
?>
--FILE--
<?php
$str = 'abc 123 #",; $foo';
echo mb_ereg_replace_callback('(\S+)', function($m){return $m[1].'('.strlen($m[1]).')';}, $str);
?>
--EXPECT--
abc(3) 123(3) #",;(4) $foo(4)

