<?php
include 'php/Dispatcher.php';
include 'php/WebThread.php';
$serv = new swoole_http_server("0.0.0.0", 9502);


$serv->on('Request',
	function($request, $response) {

		Dispatcher::handle_http_request($request, $response);

	}
);


$tick = function()
{
	echo count(Dispatcher::$free_threads)."\n";
};



$serv->on('WorkerStart',function()use($tick){
	Dispatcher::init();
//	swoole_timer_tick(1000, $tick);
});

$serv->set(array(
		'worker_num' => 1,    //worker process num
));
$serv->start();
