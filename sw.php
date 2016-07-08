<?php
$serv = new swoole_http_server("0.0.0.0", 9502);

$server = array(
		'host' => '172.16.85.30',
		'user' => 'mysqluser',
		'password' => 'mysqluser',
		'database' => 'meiyou',
);

$mysql_pool = [];
$mysqls = [];
$serv->on('Request', function($request, $response) {
	global $mysql_pool;
	echo "Request\n";
	$db_index = get_free_mysql();
	echo json_encode($db_index)."\n";
	debug_zval_dump($mysql_pool[$db_index]);
	if ($mysql_pool[$db_index])
	{
		echo "query start\n";
		$mysql_pool[$db_index]->query("select * from mz_member where mid=149;", function($db, $r)use($response,$db_index) {
			echo "query ready\n";
			$output = '';
			if ($r === false)
			{
				$output = $db->error.$db->errno;
			}
			elseif ($r === true )
			{
				$output = print_r($db,true);
			}else
			{
				$output = print_r($r,1);
			}
			$response->end($output);

			release_mysql($db_index);
			echo "query finished\n";

		}
		);
	}
	else
	{
		$response->end('busy');
	}



});

function get_free_mysql()
{
	global $mysqls;

	if (empty($mysqls))
	{
		return null;
	}
	return array_shift($mysqls);
}

function release_mysql($m)
{
	global $mysqls;
	$mysqls[] = $m;
}


$serv->on('WorkerStart',function()use($server){
	for($i=0;$i<1;$i++)
	{
		(new swoole_mysql())->connect($server, function ($db, $r) {
			global $mysql_pool,$mysqls;
			if ($r === false)
			{

			}
			else
			{
				echo "connect to mysql\n";
				$mysql_pool[] = $db;
				$mysqls[] = count($mysql_pool)-1;
				$db->query("set names utf8;",function(){});
				$db->on('Close', function($db){
					echo "MySQL connection is closed.\n";
				});
			}
		});
	}
//	swoole_timer_tick(1000, function(){
//		global $mysqls;
//		debug_zval_dump($mysqls);
//	});
});

$serv->set(array(
		'worker_num' => 1,    //worker process num
		'backlog' => 128,   //listen backlog
		'max_request' => 999999,
		'dispatch_mode'=>1,
));

$serv->start();



//$db = (new swoole_mysql());
//$db->connect($server, "conn");
//function conn($db,$r)
//{
//	echo "conn\n",var_export($r,1);
//	$db->query('show tables',"conn");
//}