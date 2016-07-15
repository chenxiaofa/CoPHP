<?php
$serv = new swoole_http_server("0.0.0.0", 9502);

$a = [];

$serv->on('Request', function($request, $response) {
	global $a;
	$a[] = $response;
});

$serv->on('WorkerStart',function()use($server){
	swoole_timer_tick(10, function(){
		global $a;
		while($p = array_pop($a))
		{
			$p->end('');
		}
	});
});
$serv->set(array(
		'worker_num' => 1,    //worker process num
));

$serv->start();

//$server = array(
//		'host' => '10.2.70.35',
//		'user' => 'xiaofa',
//		'password' => 'xiaofa',
//		'database' => 'mysql',
//);
//
//$db = (new swoole_mysql());
//$db->connect($server, "conn");
//function conn($db,$r)
//{
//	$db->query("select 1 as a1,2 as a2,3 as a3,4 as a4,5 as a5,6 as a6,7 as a7,8 as a8,9 as a9,10 as a10,11 as a11,12 as a12,13 as a13,14 as a14,15 as a15,16 as a16,17 as a17,18 as a18,19 as a19,20 as a20,21 as a21,22 as a22,23 as a23,24 as a24,25 as a25,26 as a26,27 as a27,28 as a28,29 as a29,30 as a30,31 as a31,32 as a32,33 as a33,34 as a34,35 as a35,36 as a36,37 as a37,38 as a38,39 as a39,40 as a40,41 as a41,42 as a42,43 as a43,44 as a44,45 as a45,46 as a46,47 as a47,48 as a48,49 as a49,50 as a50,51 as a51,52 as a52,53 as a53,54 as a54,55 as a55,56 as a56,57 as a57,58 as a58,59 as a59,60 as a60,61 as a61,62 as a62,63 as a63,64 as a64,65 as a65,66 as a66,67 as a67,68 as a68,69 as a69,70 as a70,71 as a71,72 as a72,73 as a73,74 as a74,75 as a75,76 as a76,77 as a77,78 as a78,79 as a79,80 as a80,81 as a81,82 as a82,83 as a83,84 as a84,85 as a85,86 as a86,87 as a87,88 as a88,89 as a89,90 as a90,91 as a91,92 as a92,93 as a93,94 as a94,95 as a95,96 as a96,97 as a97,98 as a98,99 as a99,100 as a100,101 as a101,102 as a102,103 as a103,104 as a104,105 as a105,106 as a106,107 as a107,108 as a108,109 as a109,110 as a110,111 as a111,112 as a112,113 as a113,114 as a114,115 as a115,116 as a116,117 as a117,118 as a118,119 as a119,120 as a120,121 as a121,122 as a122,123 as a123,124 as a124,125 as a125,126 as a126,127 as a127,128 as a128,129 as a129,130 as a130,131 as a131,132 as a132,133 as a133,134 as a134,135 as a135,136 as a136,137 as a137,138 as a138,139 as a139,140 as a140,141 as a141,142 as a142,143 as a143,144 as a144,145 as a145,146 as a146,147 as a147,148 as a148,149 as a149,150 as a150,151 as a151,152 as a152,153 as a153,154 as a154,155 as a155,156 as a156,157 as a157,158 as a158,159 as a159,160 as a160,161 as a161,162 as a162,163 as a163,164 as a164,165 as a165,166 as a166,167 as a167,168 as a168,169 as a169,170 as a170,171 as a171,172 as a172,173 as a173,174 as a174,175 as a175,176 as a176,177 as a177,178 as a178,179 as a179,180 as a180,181 as a181,182 as a182,183 as a183,184 as a184,185 as a185,186 as a186,187 as a187,188 as a188,189 as a189,190 as a190,191 as a191,192 as a192,193 as a193,194 as a194,195 as a195,196 as a196,197 as a197,198 as a198,199 as a199,200 as a200,201 as a201,202 as a202,203 as a203,204 as a204,205 as a205,206 as a206,207 as a207,208 as a208,209 as a209,210 as a210,211 as a211,212 as a212,213 as a213,214 as a214,215 as a215,216 as a216,217 as a217,218 as a218,219 as a219,220 as a220,221 as a221,222 as a222,223 as a223,224 as a224,225 as a225,226 as a226,227 as a227,228 as a228,229 as a229,230 as a230,231 as a231,232 as a232,233 as a233,234 as a234,235 as a235,236 as a236,237 as a237,238 as a238,239 as a239,240 as a240,241 as a241,242 as a242,243 as a243,244 as a244,245 as a245,246 as a246,247 as a247,248 as a248,249 as a249,250 as a250,251 as a251,252 as a252,253 as a253,254 as a254,255 as a255,256 as a256,257 as a257,258 as a258,259 as a259,260 as a260,261 as a261,262 as a262,263 as a263,264 as a264,265 as a265,266 as a266,267 as a267,268 as a268,269 as a269,270 as a270,271 as a271,272 as a272,273 as a273,274 as a274,275 as a275,276 as a276,277 as a277,278 as a278,279 as a279,280 as a280,281 as a281,282 as a282,283 as a283,284 as a284,285 as a285,286 as a286,287 as a287,288 as a288,289 as a289,290 as a290,291 as a291,292 as a292,293 as a293,294 as a294,295 as a295,296 as a296,297 as a297,298 as a298,299 as a299;",
//			function($db,$r){
//				var_dump($r);
//			}
//	);
//}