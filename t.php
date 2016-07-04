<?php
/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/12
 * Time: 11:46
 */
//include 'php/CoThread.php';
function a()
{
	b();
}
function b()
{
	CoThread::suspend();
}
function run()
{
	echo 123;
	a();
	echo 456;
}
$a = new \CoThread();

$a->start();