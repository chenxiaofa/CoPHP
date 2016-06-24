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
	function test()
	{

	}
	protected function run()
	{
		print_r(debug_backtrace());
		print_r($this);
		echo 'ct:1-1',"\n";
		echo 'ct:1-3',"\n";
		$this->status = CoThread::STATUS_DEAD;
		print_r($this);
	}
}
//class ct2 extends CoThread
//{
//
//	public function run()
//	{
//		echo 'ct:2-2',"\n";
//		$this->suspend();
//		echo 'ct:2-4',"\n";
//		$this->status = CoThread::STATUS_DEAD;
//	}
//}
//function run()
//{
//
//	echo "main run","\n";
//}
//call_user_func("run");
$ct = (new ct1());

$ret = $ct->start();
echo 'main',"\n";
debug_zval_dump($ret);

exit;

$ct->resume();
//
//$threads = array(new ct1(),new ct2());
//while(1)
//{
//	/** @var CoThread $r */
//	foreach($threads as $k=>$r)
//	{
//		$r->run();
//		if ($r->status == CoThread::STATUS_DEAD)
//		{
//			unset($threads[$k]);
//		}
//	}
//
//}