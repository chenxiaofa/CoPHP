<?php

/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/7/11
 * Time: 17:23
 */
class Dispatcher
{
	const MAX_THREAD_NUM = 200;
	const START_THREAD_NUM = 0;
	private static $threads = [];
	public static $free_threads = [];
	private static $thread_num = 0;
	private function __construct()
	{

	}

	public static function init()
	{
		for($i=0 ; $i< self::START_THREAD_NUM; $i++)
		{
			self::incr_thread();
		}

	}

	public static function incr_thread()
	{
		$thread = new WebThread();
		self::$free_threads[] = $thread;
		self::$threads[] = $thread;
		self::$thread_num++;
	}

	public static function run_co_thread($thread)
	{
		/** @var WebThread $thread */
		if ($thread->status == $thread::STATUS_SUSPEND)
		{
			$thread->resume();
		}
		if ($thread->status == $thread::STATUS_DEAD)
		{
			self::$free_threads[] = $thread;
		}
//		echo "run\n";
		return;
	}

	public static function handle_http_request($request,$response)
	{
		while(empty(self::$free_threads))
		{
			if (self::$thread_num >= self::MAX_THREAD_NUM)
			{
				$response->end('busy');
				return;
			}
			self::incr_thread();
		}
		/** @var WebThread $thread */
		$thread = array_pop(self::$free_threads);
		$thread->reset($request,$response);
		self::run_co_thread($thread);
	}




}
