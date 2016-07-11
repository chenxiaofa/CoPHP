<?php
include 'php/Dispatcher.php';
include 'php/WebThread.php';
$serv = new swoole_http_server("0.0.0.0", 9502);


$serv->on('Request',
	function($request, $response) {
		Dispatcher::add_http_request($request, $response);
	}
);


$tick = function()
{
	Dispatcher::roll();
};



$serv->on('WorkerStart',function()use($tick){
	swoole_timer_tick(1, $tick);
	swoole_timer_tick(1000, function(){
		echo count(Dispatcher::$free_threads)."\n";
		echo count(Dispatcher::$http_requests)."\n";
	});

});

Dispatcher::init();
$serv->set(array(
		'worker_num' => 1,    //worker process num
));
$serv->start();
