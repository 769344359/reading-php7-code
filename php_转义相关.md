```
(gdb) bt
#0  zend_scan_escape_string (zendlval=0x7fffffffa070, str=0x7ffff7ff600d "\\\\\";\n$b ='\\\\a';\necho $a+$b;\n\n", len=2, quote_type=34 '"') at Zend/zend_language_scanner.l:874
#1  0x00000000009adc71 in lex_scan (zendlval=0x7fffffffa070) at Zend/zend_language_scanner.l:2044
#2  0x00000000009d3857 in zendlex (elem=0x7fffffffa0e0) at /home/vagrant/php-7.2.5/Zend/zend_compile.c:1721
#3  0x00000000009a20df in zendparse () at /home/vagrant/php-7.2.5/Zend/zend_language_parser.c:4227
#4  0x00000000009a8f82 in zend_compile (type=2) at Zend/zend_language_scanner.l:585
#5  0x00000000009a91cd in compile_file (file_handle=0x7fffffffd270, type=8) at Zend/zend_language_scanner.l:635
#6  0x000000000077c7d8 in phar_compile_file (file_handle=0x7fffffffd270, type=8) at /home/vagrant/php-7.2.5/ext/phar/phar.c:3320
#7  0x0000000000a06883 in zend_execute_scripts (type=8, retval=0x0, file_count=3) at /home/vagrant/php-7.2.5/Zend/zend.c:1490
#8  0x000000000096ef77 in php_execute_script (primary_file=0x7fffffffd270) at /home/vagrant/php-7.2.5/main/main.c:2590
#9  0x0000000000af0dcb in do_cli (argc=2, argv=0x1631aa0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1011
#10 0x0000000000af1f7d in main (argc=2, argv=0x1631aa0) at /home/vagrant/php-7.2.5/sapi/cli/php_cli.c:1404
```
