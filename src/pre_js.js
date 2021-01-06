/********************************************************************/
/*                                                                  */
/*  pre_js.js     JavaScript part to be included by Emscripten.     */
/*  Copyright (C) 2020  Thomas Mertes                               */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/pre_js.js                                       */
/*  Changes: 2020  Thomas Mertes                                    */
/*  Content: JavaScript part to be included by Emscripten.          */
/*                                                                  */
/********************************************************************/

var mapIdToWindow = {};
var mapIdToCanvas = {};
var mapIdToContext = {};
var currentWindowId = 0;  
if (typeof document !== "undefined") {
  var scripts = document.getElementsByTagName('script');
  var index = scripts.length - 1;
  var myScript = scripts[index];
  var src = myScript.src;
  var bslash = String.fromCharCode(92);
  var n = src.search(bslash + '?');
  var programPath = myScript.src;
  var queryString = '';
  if (n !== -1) {
    queryString = programPath.substring(n + 1);
    programPath = programPath.substring(0, n);
  }
  var arguments = queryString.split('&');
  for (var i = 0; i < arguments.length; i++) {
    arguments[i] = decodeURIComponent(arguments[i])
  }
  var Module = {
    'thisProgram': programPath,
    'arguments': arguments
  }
} else if (typeof Module !== "undefined") {
  // console.log('Module exists');
  // var Module = {
  //   'print': function(text) { process.stdout.write(text) }
  // };
}
