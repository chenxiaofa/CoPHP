<?php

/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/6/13
 * Time: 19:07
 */
class CoThread
{
	const STATUS_SUSPEND = 0;
	const STATUS_RUNNING = 1;
	const STATUS_DEAD = 2;

	public $status = 0;
	public static function suspend(){}

	/**
	 * @return static
	 */
	public static function get_current_cothread(){}
	public function start(){}
	public function resume(){}
	public function reset(){}

}