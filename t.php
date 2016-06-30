<?php
/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/12
 * Time: 11:46
 */
//include 'php/CoThread.php';

class ct1 extends CoThread
{
	protected function run()
	{
		usleep(10000);
		$this->suspend();
	}
}

while(1)
{
	(new ct1())->start();
}
//$a = array();
//while(1)
//{
//	$a[] = new ct1();
//	echo memory_get_usage(1),'----',$i++,"\n";
//	if ($i == 400) break;
//}
//
//foreach($a as $k=>$v)
//{
//	unset($a[$k]);
//	echo memory_get_usage(1),'----',$k,"\n";
//}
//
//$a = array();$i=0;
//while(1)
//{
//	$a[] = new ct1();
//	echo memory_get_usage(1),'----',$i++,"\n";
//	if ($i == 400) break;
//}
