var fs = require("fs");
var path = require('path');
var g_fileList = [];

Array.prototype.contains = function(obj) {
    var i = this.length;
    while (i--) {
        if (this[i] === obj) {
            return true;
        }
    }
    return false;
}
function walk(dir){  
    var dirList = fs.readdirSync(dir);

    dirList.forEach(function(item){
		var file = dir + '/' + item;
		if(fs.statSync(file).isFile()){
			var extname = path.extname(file);
			if (extname == ".js") {
				g_fileList.push(file);
			}
        }
    });

    /*dirList.forEach(function(item){
		var file = dir + '/' + item;
        if(fs.statSync(file).isDirectory()){
			walk(file);
        }
    });*/
}

var exec = require('child_process').exec;
var execSync = function(cmd, options) {
    //for linux use ; instead of &&
    //execute your command followed by a simple echo 
    //to file to indicate process is finished
	var task = exec(cmd, options, 
		function (error, stdout, stderr) {
			console.log("exit");
			//fs.writeFileSync("C:/MyProject/llvm-3.4.2/tools/jslang/sync.txt", "done");
		});
	console.log("pid:"+task.pid+'\n');
    while (true) {
        //consider a timeout option to prevent infinite loop
        //NOTE: this will max out your cpu too!
        try {
            var status = fs.readFileSync('sync.txt', 'utf8');

            if (status.trim() == "done") {
                var res = fs.readFileSync("stdout.txt", 'utf8');
                fs.unlinkSync("stdout.txt"); //cleanup temp files
                fs.unlinkSync("sync.txt");
                return res;
            }
        } catch(e) { } //readFileSync will fail until file exists
    }
};

function genObject(js_file, obj_file, log_file_handle) {
	var compileCmd = "C:/MyProject/llvm-3.4.2/bin/Debug/jslang.exe ";
	var compileDir = "C:/MyProject/llvm-3.4.2/bin/Debug";
	console.log(compileCmd+js_file+" "+obj_file);
	exec(compileCmd+js_file+" "+obj_file, {}, 
		function (error, stdout, stderr) {
			//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
			//fs.writeSync(log_file_handle, 'stderr: \n' + stderr + '\n');
			console.log('stdout: \n' + stdout + '\n');
			console.log('stderr: \n' + stderr + '\n');
			if (error !== null) {
				//fs.writeSync(log_file_handle, 'compile error: \n' + error + '\n');
				console.log('compile error: \n' + error + '\n');
			}
		});
}

function genBinary(obj_file, exe_file, log_file_handle) {
	var linkCmd = "C:/MyProject/llvm-3.4.2/tools/jslang/jslang_link.bat ";
	var linkDir = "C:/MyProject/llvm-3.4.2/tools/jslang";
	console.log(linkCmd+obj_file+" "+exe_file);
	exec(linkCmd+obj_file+" "+exe_file, {cwd: "C:/MyProject/llvm-3.4.2/tools/jslang"}, 
		function (error, stdout, stderr) {
			//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
			//fs.writeSync(log_file_handle, 'stderr: \n' + stderr + '\n');
			console.log('stdout: \n' + stdout + '\n');
			console.log('stderr: \n' + stderr + '\n');
			if (error !== null) {
				//fs.writeSync(log_file_handle, 'link error: \n' + error + '\n');
				console.log('link error: \n' + error + '\n');
			}
		});
}

function runBinary(exe_file, log_file_handle) {
	console.log(exe_file);
	exec(exe_file, {cwd: "C:/MyProject/llvm-3.4.2/tools/jslang"},
		function (error, stdout, stderr) {
			//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
			//fs.writeSync(log_file_handle, 'stderr: \n' + stderr + '\n');
			console.log('stdout: \n' + stdout + '\n');
			console.log('stderr: \n' + stderr + '\n');
			if (error !== null) {
				//fs.writeSync(log_file_handle, 'run error: \n' + error + '\n');
				console.log('run error: \n' + error + '\n');
			}
		});
}

function genObject_sync(js_file, obj_file, log_file_handle) {
	var compileCmd = "C:/MyProject/llvm-3.4.2/bin/Debug/jslang.exe ";
	var compileDir = "C:/MyProject/llvm-3.4.2/bin/Debug";
	console.log(compileCmd+js_file+" "+obj_file);
	var stdout = execSync(compileCmd+js_file+" "+obj_file, {});
	//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
	console.log('stdout: \n' + stdout + '\n');
}

function genBinary_sync(obj_file, exe_file, log_file_handle) {
	var linkCmd1 = '@call "c:/Program Files (x86)/Microsoft Visual Studio 10.0/VC/vcvarsall.bat" x86 && link.exe ';
	var linkCmd2 = ' jslang.dir/Debug/jsextern.obj jslang.dir/Debug/allinone_for_link.obj "pcred.lib" "pthreadVC2_Debug.lib" /DEBUG /SUBSYSTEM:CONSOLE /STACK:"10000000" /OUT:'
	var linkDir = "C:/MyProject/llvm-3.4.2/tools/jslang";
	console.log(linkCmd1+obj_file+linkCmd2+exe_file);
	var stdout = execSync(linkCmd1+obj_file+linkCmd2+exe_file, {cwd: "C:/MyProject/llvm-3.4.2/tools/jslang"});
	//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
	console.log('stdout: \n' + stdout + '\n');
}

function runBinary_sync(exe_file, log_file_handle) {
	console.log(exe_file);
	var stdout = execSync(exe_file, {cwd: "C:/MyProject/llvm-3.4.2/tools/jslang"});
	//fs.writeSync(log_file_handle, 'stdout: \n' + stdout + '\n');
	console.log('stdout: \n' + stdout + '\n');
}

var testDir = "C:/MyProject/echo-js/test";
var outDir = "C:/MyProject/llvm-3.4.2/tools/jslang/out";

if (!fs.existsSync("out")) {
	fs.mkdirSync("out");
}

walk(testDir);
//console.log(g_fileList);
for (var i = 0; i < g_fileList.length; i++) {
	var js_file = g_fileList[i];
	var relativePath = js_file.substring(testDir.length);
	var obj_file = outDir + relativePath + ".obj";
	var exe_file = outDir + relativePath + ".exe";
	var log_file = outDir + relativePath + ".log";
	var log_file_handle;// = fs.openSync(log_file, 'a+');
	console.log("js_file:"+js_file);
	console.log("obj_file:"+obj_file);
	console.log("exe_file:"+exe_file);
	console.log("log_file:"+log_file);
	
	//clean files first
	if (fs.existsSync(obj_file)) {
		fs.unlinkSync(obj_file);
	}
	if (fs.existsSync(exe_file)) {
		fs.unlinkSync(exe_file);
	}
	if (fs.existsSync(log_file)) {
		fs.unlinkSync(log_file);
	}

	//fs.writeSync(log_file_handle, '==========compiling==========\n');
	console.log('==========compiling==========\n');
    genObject_sync(js_file, obj_file, log_file_handle);
	if (fs.existsSync(obj_file)) {
		//fs.writeSync(log_file_handle, '==========linking============\n');
		console.log('==========linking============\n');
		genBinary_sync(obj_file, exe_file, log_file_handle);
		if (fs.existsSync(exe_file)) {
			//fs.writeSync(log_file_handle, '==========running============\n');
			console.log('==========running============\n');
			runBinary_sync(exe_file, log_file_handle);
		} else {
			//fs.writeSync(log_file_handle, '==========linking failed=====\n');
			console.log('==========linking failed=====\n');
		}
	} else {
		//fs.writeSync(log_file_handle, '==========compiling failed===\n');
		console.log('==========compiling failed===\n');
	}

	//fs.closeSync(log_file_handle);

	if (i >= 0)
	{
	    //break;
	}
}