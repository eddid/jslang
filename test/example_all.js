/////////////////////
//nesting function
/////////////////////
function fib(x) {
  if (x == 1)
    return 1;
  else if (x == 2)
    return 1;
  else
    return fib(x-1)+fib(x-2);
}

function fib1(x) {
  if (x <= 2)
    return 1;
  else
    return fib1(x-1)+fib1(x-2);
}

function fib2(x) {
  if (x <= 2)
    return 1;

    return fib2(x-1)+fib2(x-2);
}

a=1;
a+=39;
console.log(fib(a));
console.log(fib1(a));
console.log(fib2(a));

/////////////////////
//for
/////////////////////
function for_proc(a) {
	var b = 0;
	for (var i = 0; i < a; i++) {
		b += i;
	}
	return b;
}
console.log(for_proc(3));

var str;
str.msg = "test";
str.len = 4;
printf(str.msg);
console.log(str.len);

/////////////////////
//logical options
/////////////////////
var a0 = 0;
var a1 = 1;
var a2 = 2;
var a3 = 3;
var a4 = 4;
var a5 = 5;
var a6 = 6;
var a7 = 7;
var a8 = 8;
var a9 = 9;
var d = ((a1 && a9) && ((a2 && a0) || (a4 && a3 && a5)) && (a6 || a7 && a8));

/////////////////////
//switch
/////////////////////
var a = 0;
switch (a)
{
case null:
    console.log("null");
    break;
case undefined=1:
    console.log("undefined");
    break;
case a:
    console.log("number1");
    //break;
default:
    console.log("unknown");
    break;
case "0":
    console.log("string");
    break;
case false:
    console.log("boolean");
    break;
case 0:
    console.log("number");
    break;
}

/////////////////////
//regular expression
/////////////////////
var re1 = /\w+/;
var re2 = /ab+c/i;
var result = re1.exec("cdb,Bdbsbz");
if (result != null)
{
    console.log("match:" + result[0]);
}
else
{
    console.log("no match !");
}

/////////////////////
//for in
/////////////////////
var obj = ["a", "b", "c"];
var obj1 = { 100: "a", 2: "b", 7: "c"};
for (var prop in obj) {
    console.log("o." + prop + " = " + obj[prop]);
}
var obj1 = ["a", "b", "c"];
var obj2 = { p1: "a", p2: "b", p3: "c"};
console.log("obj1:"+obj1.length);
for (var prop1 in obj1) {
    console.log(obj1[prop1]);
}
console.log("obj2:"+obj2.length);
for (var prop2 in obj2) {
    console.log(obj2[prop2]);
}