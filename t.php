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
class WebThread extends CoThread
{
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
				print_r(CoThread::yield());
			}
		);
	}

	public function run()
	{
		debug_zval_dump($this);
	}


}

while(1)
{
		$a = new WebThread();
		$a->resume();
}