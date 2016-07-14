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
	const MAX_THREAD_NUM = 50;
	private static $threads = [];
	public static $free_threads = [];
	private static $high_threads = [];
	private static $hangup_thread = [];
	public static $http_requests = [];

	private function __construct()
	{

	}

	public static function init()
	{
		for($i=0 ; $i< self::MAX_THREAD_NUM; $i++)
		{
			$thread = new WebThread();
			self::$free_threads[] = $thread;
			self::$threads[] = $thread;
		}

	}


	public static function add_http_request($request,$response)
	{
//		static $count = 0;
		self::$http_requests[] = [$request,$response];
//		echo "add_http_request count = ".++$count."\n";
	}


	public static function add_high_thread($thread)
	{
		self::$high_threads[] = $thread;
	}


	public static function roll()
	{
		while(!empty(self::$high_threads))
		{
//			echo "high_threads\n";
			$thread = array_shift(self::$high_threads);
			$thread->resume();
			if ($thread->status == $thread::STATUS_DEAD)
			{
				self::$free_threads[] = $thread;
			}
			return;
		}
		while(!empty(self::$hangup_thread))
		{
			$thread = array_shift(self::$hangup_thread);
			$thread->resume();
			if ($thread->status == $thread::STATUS_DEAD)
			{
				self::$free_threads[] = $thread;
			}
			return;
		}
		while(!empty(self::$free_threads) && !empty(self::$http_requests))
		{
			/** @var WebThread $thread */
			$thread = array_pop(self::$free_threads);
			$p = array_shift(self::$http_requests);
			$thread->reset($p[0],$p[1]);
			self::$hangup_thread[] = $thread;
		}


	}
}
