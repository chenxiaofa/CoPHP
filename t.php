<?php
/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/12
 * Time: 11:46
 */
//include 'php/CoThread.php';
include "cophp.php";
$_GET = array(1,2,3);
function a()
{
	b();
}
function b()
{
	echo 123;
	CoThread::suspend();
	echo 789;
}
function run()
{
	a();

//	(new a())->a();
}
//$a='asdasd';
//$f = function()use($a)
//{
//	print_r($a);
//};
//$f();exit;
/*
$b = 1;
$a = new \CoThread(
	function()
	{
		run();
	}
);

$a->start();
*/


//while(1)
//{
//	$a = array();
//	$a[] = new \CoThread(
//		function()
//		{
//			include "cophp.php";
//			(new a())->a();
//		}
//	);
//
//	foreach($a as $c)
//	{
//		$c->start();
//	}
//
//}
//

$runable = function()use($b)
{
	run();

};

//while(1)
{

	$a = new \CoThread($runable);
	$a->start();
	echo 456;
	$a->resume();
}
//while(1)
//{
//	$a = array();
//	$c = function(){
//		static $p = 0;
//	};
//	$a[] = new \CoThread($c);
//	debug_zval_dump($c);
//	foreach($a as $c)
//	{
//		$c->start();
//	}
//}
