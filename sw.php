<?php
$serv = new swoole_http_server("0.0.0.0", 9502);
$cothreads = [];
$free_cothreads = [];
$running_queue = [];


class mCothread extends CoThread
{
	public $request = null;
	public $response = null;
	public function reset($request,$response)
	{
		$this->request = $request;
		$this->response = $response;
	}
}


//for($i=0;$i<2000;$i++)
//{
//	$free_cothreads[] = new mCothread(function()
//	{
//		$self = mCothread::get_current_cothread();
//		$a = 'hello';
//		$self->suspend();
//		$b = 'world';
//		$self->response->end($a.' '.$b);
//	});
//}
$serv->on('Request', function($request, $response)use(&$running_queue) {
	$co = get_free_cothread();
	$co->reset($request, $response);
	$running_queue[] = $co;
});




function get_free_cothread()
{
//	global $free_cothreads;
//	return array_shift($free_cothreads);

	return new mCothread(function()
		{
			$self = mCothread::get_current_cothread();
			$a = 'hello';
			$self->suspend();
			$b = 'world';
			$self->response->end($a.' '.$b);
		}
	);

}


$tick = function()use(&$running_queue,&$free_cothreads)
{
	static $i=0;
	static $count = 0;
	$count += count($running_queue);
	if($i++ %1000 == 0)
	{
		echo "count:".$count,"\n";
		$count = 0;
	}
	/**/
	while(!empty($running_queue))
	{
		$t = array_shift($running_queue);
		$t->resume();
		if ($t->status == mCothread::STATUS_SUSPEND)
		{
			$running_queue[] = $t;
		}
	}


};

$serv->on('WorkerStart',function()use($tick){
	swoole_timer_tick(1, $tick);
});


$serv->start();

