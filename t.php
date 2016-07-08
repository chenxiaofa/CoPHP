<?php
class mCothread extends CoThread
{
	public $request = null;
	public $response = null;
	public $sql = '';
	public function reset1($request,$response)
	{
		$this->request = $request;
		$this->response = $response;
	}

	/**
	 * @param $sql
	 */
	public function query($sql)
	{
		$this->sql = $sql;
	}
}

$a = new mCothread(
		function()
		{
			$self = mCothread::get_current_cothread();
			echo "do_request\n";
			run();
		}
);

function run()
{
	a();
}
function a()
{
	b();
}
function b()
{
	c();
}
function c()
{
	echo 123;
	CoThread::suspend();
	echo 789;
}
while(1)
{

		$a->resume();
	print_r($a);
		echo "\n";
		if ($a->status == $a::STATUS_DEAD)
		{
			$a->reset();
		}
}