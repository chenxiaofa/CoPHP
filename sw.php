<?php
$serv = new swoole_http_server("0.0.0.0", 9502);

class tt{
	public $req = null;
}
$a = [];
$serv->on('Request', function($request, $response) {
	global $a;
	$t = new tt();
	$t->req = $request;
	$a[] = $t;
	$response->end(1);

});

$serv->on('WorkerStart',function()use($server){
	swoole_timer_tick(100, function(){
		global $a;
		foreach($a as $t)
		{
			$t->req = null;
		}
		$a = [];
	});
});
$serv->set(array(
		'worker_num' => 1,    //worker process num
));

$serv->start();
