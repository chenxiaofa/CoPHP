<?php
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
while(1)
{

		$a = new mCothread(function()
		{
			$self = mCothread::get_current_cothread();
			echo "do_request\n";
		});
		$a->start();
//		$a->resume();


}