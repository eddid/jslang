#!/usr/bin/env node
var fs = require ("fs");

var atom_def = fs.readFileSync (process.argv[2], "utf-8");

var atom_lines = atom_def.split ("\n");
var ucs2_lines = [];
var utf8_lines = [];
var declare_lines = [];
var atom_names = [];

for (var i = 0, e = atom_lines.length; i < e; i ++) {
  var atom = null;
  var atom_name = null;

  var match  = atom_lines[i].match(/^EJS_ATOM\((.*)\)$/);
  var match2 = atom_lines[i].match(/^EJS_ATOM2\((.*),(.*)\)$/);
  var match3 = atom_lines[i].match(/^EJS_ATOM2\(,(.*)\)$/);

  if (match) {
    atom = match[1];
    atom_name = match[1];
  }
  else if (match2) {
    atom = match2[1];
    atom_name = match2[2];
  }
  else if (match3) {
    atom = "";
    atom_name = match3[1];
  }

  if (atom != null) {
    // output the ucs2 literal for the atom
    var utf8_line = "const jschar _ejs_buffer_" + atom_name + "[] = { \"" + atom + "\" };";
    utf8_lines.push(utf8_line);
    var ucs2_line = "const jschar _ejs_buffer_" + atom_name + "[] = { L\"" + atom + "\" };";
    ucs2_lines.push(ucs2_line);

    declare_lines.push("static EJSPrimString _ejs_primstring_" + atom_name + " = { /*.gc_header =*/ (EJS_STRING_FLAT<<EJS_GC_USER_FLAGS_SHIFT), /*.length =*/ " + atom.length + ", /*.hash =*/ 0, /*.data =*/ 0 };");
    declare_lines.push("ejsval _ejs_atom_" + atom_name + ";");

    atom_names.push(atom_name);
  }
  else {
    utf8_lines.push(atom_lines[i]);
    ucs2_lines.push(atom_lines[i]);
  }
}
console.log("#if defined(JSLANG_UTF8)");
console.log (utf8_lines.join('\n'));
console.log("#else");
console.log (ucs2_lines.join('\n'));
console.log("#endif\n");
console.log (declare_lines.join('\n'));

console.log ("\nstatic void _ejs_init_static_strings() {");
for (var an = 0, ae = atom_names.length; an < ae; an ++) {
  var atom = atom_names[an];
  console.log ("    _ejs_primstring_" + atom + ".data.flat = (jschar*)_ejs_buffer_" + atom + ";");
}
for (var an = 0, ae = atom_names.length; an < ae; an ++) {
  var atom = atom_names[an];
  console.log ("    _ejs_atom_" + atom + " = STRING_TO_EJSVAL((EJSPrimString*)&_ejs_primstring_" + atom + ");");
}
console.log ("}");
