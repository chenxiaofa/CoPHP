<?php
/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/7/11
 * Time: 17:28
 */

function run()
{
	echo "123";
}
$_POST['123'] = 123;
class WebThread1 extends CoThread
{
	private $_GET=[];
	private $_POST=[];
	private $_COOKIE=[];
	private $_SERVER=[];
	private $_FILES=[];
	/** 构造函数
	 * @param null $callback
	 */
	public function __construct($callback=null)
	{
//		parent::__construct([$this,'run']);
//		parent::__construct('run');
		parent::__construct(
			function()
			{
				global $yy;
//				echo 123;exit;
				
				$_POST = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
				echo "CoThread:";
				debug_zval_dump($_POST);
				echo "\n";
				$yy[] =$_POST;
				self::yield();

				echo "CoThread:";
				debug_zval_dump($_POST);
				echo "\n";
			}
		);
	}


	public function resume()
	{
		$_TMP_GET = &$_GET;
		$_TMP_POST = &$_POST;
		$_TMP_COOKIE = &$_COOKIE;
		$_TMP_SERVER = &$_SERVER;
		$_TMP_FILES = &$_FILES;

		$_POST = &$this->_POST;
		$_GET = &$this->_GET;
		$_COOKIE = &$this->_COOKIE;
		$_SERVER = &$this->_SERVER;
		$_FILES = &$this->_FILES;

		parent::resume();

		$this->_POST = &$_POST;
		$this->_GET = &$_GET;
		$this->_COOKIE = &$_COOKIE;
		$this->_SERVER = &$_SERVER;
		$this->_FILES = &$_FILES;

		$_GET = &$_TMP_GET;
		$_POST = &$_TMP_POST;
		$_COOKIE = &$_TMP_COOKIE;
		$_SERVER = &$_TMP_SERVER;
		$_FILES = &$_TMP_FILES;
	}
}
//print_r($_COOKIE);
//while(1)
{
	echo "Main:";
	debug_zval_dump($_POST);
	$a = new WebThread1();
	$a->resume();
//	echo "Main:";
	debug_zval_dump($_COOKIE);
	$a->resume();
	echo memory_get_usage(1),"\n";

}

register_shutdown_function(function(){echo 123;});