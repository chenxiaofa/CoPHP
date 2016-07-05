<?php
class a
{
	function a()
	{
		CoThread::suspend();
		(new b())->b();
	}
}
class b
{
	function b()
	{
		CoThread::suspend();
		(new cc())->c();
	}
}

class cc
{
	function c()
	{
		CoThread::suspend();
		(new d())->d();
	}
}

class d
{
	function d()
	{
		CoThread::suspend();
		(new e())->e();
	}
}
class e
{
	function e()
	{
		CoThread::suspend();
	}
}
