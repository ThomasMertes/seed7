/********************************************************************/
/*                                                                  */
/*  pre_js.js     JavaScript part to be included by Emscripten.     */
/*  Copyright (C) 2020, 2021  Thomas Mertes                         */
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
/*  Changes: 2020, 2021  Thomas Mertes                              */
/*  Content: JavaScript part to be included by Emscripten.          */
/*                                                                  */
/********************************************************************/

var mapIdToWindow = {};
var mapIdToCanvas = {};
var mapIdToContext = {};
var currentWindowId = 0;
var callbackList = [];
function registerCallback (callback) {
    // console.log("register callback " + callbackList.length.toString());
    callbackList.push(callback);
}
function executeCallbacks () {
    for (let i = 0; i < callbackList.length; i++) {
        // console.log("execute callback " + i.toString());
        callbackList[i](1114511); // K_NONE
    }
    callbackList = [];
}
var callbackList2 = [];
function registerCallback2 (callback) {
    // console.log("register callback " + callbackList.length.toString());
    callbackList2.push(callback);
}
function executeCallbacks2 () {
    for (let i = 0; i < callbackList2.length; i++) {
        // console.log("execute callback2 " + i.toString());
        callbackList2[i](["", null]);
    }
    callbackList2 = [];
}
if (typeof document !== "undefined") {
  let scripts = document.getElementsByTagName('script');
  let index = scripts.length - 1;
  let myScript = scripts[index];
  let src = myScript.src;
  let bslash = String.fromCharCode(92);
  let questionMarkPos = src.search(bslash + '?');
  let programPath = myScript.src;
  let queryString = '';
  if (questionMarkPos !== -1) {
    queryString = programPath.substring(questionMarkPos + 1);
    programPath = programPath.substring(0, questionMarkPos);
  }
  let arguments = queryString.split('&');
  for (let i = 0; i < arguments.length; i++) {
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
