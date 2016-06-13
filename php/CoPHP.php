<?php

/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/13
 * Time: 19:07
 */
class CoPHP
{
	const STATUS_HANGUP = 0;
	const STATUS_RUNING = 0;
	const STATUS_SUSPEND = 0;

	public $status = 0;
	public function suspend(){}
	public function run(){}

	/**
	 * @param $callable
	 * @return static
	 */
	static function create($callable){}
}