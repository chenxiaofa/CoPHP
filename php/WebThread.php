<?php

/**
 * 珠海魅族科技有限公司
 * Copyright (c) 2012 - 2013 meizu.com ZhuHai Inc. (http://www.meizu.com)
 * User: 陈晓发
 * Date: 2016/7/11
 * Time: 17:28
 */
class WebThread extends CoThread
{
	public $request = null;
	public $response = null;
	protected $db = null;

	public static function mysql_query($sql , $db = null)
	{

		$self = self::running();
		if ($db == null)
		{
			$db = $self->get_db();
		}

		$result = null;
		$s = microtime(1);
		$e = 0;
		$db->query($sql,
			function($db,$r)use(&$result,$s,$self)
			{
//				echo "query:".(microtime(1) - $s)."\n";
				$result = $r;
				Dispatcher::add_high_thread($self);
			}
		);
		self::yield();
//		echo "query cothread resume:".(microtime(1) - $s)."\n";
		return $result;
	}

	/** 构造函数 */
	public function __construct()
	{
		parent::__construct([$this,'run']);
	}

	public function run()
	{

		$result = WebThread::mysql_query('select * from mz_member where mid=149;');
		print_r($result);
//		print_r($db);
	}

	public function reset($request,$response)
	{
		$this->request = $request;
		$this->response = $response;
		parent::reset();
	}

	public function get_db()
	{
		if ($this->db && $this->db->connected == 1)
		{
			return $this->db;
		}
		if ($this->db === null)
		{
			$this->db = new swoole_mysql();
		}

		$server = array(
				'host' => '172.16.85.30',
				'user' => 'mysqluser',
				'password' => 'mysqluser',
				'database' => 'meiyou',
		);
		$thread = $this;
		$this->db->connect($server,function($db,$r)use($thread){
			if ($r)
			{
				$db->query('set names utf8;',function()use($thread){
					Dispatcher::add_high_thread($thread);
				});
			}
		});
		self::yield();
		return $this->db;

	}

	public function resume()
	{
		parent::resume();
		if ($this->status === self::STATUS_DEAD)
		{
			$this->_release();
		}
	}

	private function _release()
	{
//		if ($this->db !== NULL)
//		{
//			release_mysql($this->db);
//			$this->db = NULL;
//		}
		$this->response = null;
		$this->request = null;

	}
}