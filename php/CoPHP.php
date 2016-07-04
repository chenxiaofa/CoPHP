<?php

/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/13
 * Time: 19:07
 */
abstract class CoThread
{

	const STATUS_NEW = 0;
	const STATUS_RUNNABLE = 1;
	const STATUS_RUNNING = 2;
	const STATUS_SUSPEND = 3;
	const STATUS_DEAD = 4;

	public $status = 0;
	public static function suspend(){}
	public function start(){}
	public function resume(){}
	protected abstract  function run();

}