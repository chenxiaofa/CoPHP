<?php
/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/12
 * Time: 11:46
 */
//function a()
//{
//	$i = 0;
//	CoPHP::record();
//	echo $i++,"\n";
//	c();
//	echo 7;
//}
//function b()
//{
//	$a=[];
//}
//function c()
//{
//	$i = 0;
//	echo "\t",$i++,"\n";
//	CoPHP::suspend();
//}
//a();
//


$cophp = new CoPHP(
	function()
	{
		echo 123;
		return 'called';
	}
);

echo "\n";

debug_zval_dump($cophp->callable);
debug_zval_dump($cophp->run());