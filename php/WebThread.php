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
	public $flag = 0;

	public $request = null;
	public $response = null;
	protected $db = null;

	protected $thread_num = 0;
	protected $request_num = 0;
	//全局变量
	private $_GET=[];
	private $_POST=[];
	private $_COOKIE=[];
	private $_SERVER=[];
	private $_FILES=[];

	public static function mysql_query($sql , $db = null)
	{

		$thread = self::running();
		if ($db == null)
		{
			$db = $thread->get_db();
		}

		$result = null;
		$db->query($sql,
			function($db,$r)use(&$result,$thread)
			{
				$result = $r;
				Dispatcher::run_co_thread($thread);
			}
		);
		self::yield();
		return $result;
	}

	/** 构造函数 */
	public function __construct()
	{
		static $n=0;
		$this->thread_num = ++$n;
		parent::__construct([$this,'run']);
	}

	public function run()
	{
		$result  = self::mysql_query('select * from mz_member;');

		$this->response->end(json_encode($result));
	}

	public function reset($request,$response)
	{

		$this->request = $request;
		$this->response = $response;
		$this->_POST = [];
		$this->_GET = [];
		$this->_COOKIE = [];
		$this->_SERVER = [];
		$this->_FILES = [];
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
					Dispatcher::run_co_thread($thread);
				});
			}
		});
		self::yield();
		return $this->db;

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
		if ($this->status === self::STATUS_DEAD)
		{
			$this->_release();
		}

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

	private function _release()
	{

//		if ($this->db !== NULL)
//		{
//			release_mysql($this->db);
//			$this->db = NULL;
//		}
//		$this->response = null;
//		$this->request = null;

	}
}