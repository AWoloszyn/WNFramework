/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

static const char* common_rcss =
    "body\n"
    "{\n"
    "	font-family: Fira Code;\n"
    "	z-index: top;\n"
    "	font-size: 12px;\n"
    "	color: black;\n"
    "	padding-top: 30px;\n"
    "}\n"
    "div, h1, h2, h3, h4, p\n"
    "{\n"
    "	display: block;\n"
    "}\n"
    "em\n"
    "{\n"
    "	font-style: italic;\n"
    "}\n"
    "h1\n"
    "{\n"
    "	position: absolute;\n"
    "	top: 0px;\n"
    "	height: 22px;\n"
    "	padding: 4px;\n"
    "	color: white;\n"
    "	background-color: #888;\n"
    "	font-size: 15px;\n"
    "}\n"
    "h2\n"
    "{\n"
    "	background-color: #ddd;\n"
    "	border-width: 1px 0px;\n"
    "	border-color: #888;\n"
    "}\n"
    "h3\n"
    "{\n"
    "	margin-top: 1px;\n"
    "	color: red;\n"
    "}\n"
    "h4\n"
    "{\n"
    "	color: #cc0000;\n"
    "}\n"
    "handle#position_handle\n"
    "{\n"
    "	height: 100%;\n"
    "	width: 100%;\n"
    "}\n"
    "div#close_button\n"
    "{\n"
    "	margin-left: 10px;\n"
    "	float: right;\n"
    "	width: 18px;\n"
    "	color: black;\n"
    "	background-color: #ddd;\n"
    "	border-width: 1px;\n"
    "	border-color: #666;\n"
    "	text-align: center;\n"
    "}\n"
    "div#content\n"
    "{\n"
    "	position: relative;\n"
    "   width: auto;\n"
    "   height: 100%;\n"
    "	overflow: auto;\n"
    "	background: white;\n"
    "	border-width: 2px;\n"
    "	border-color: #888;\n"
    "	border-width-top: 0px;\n"
    "}\n"
    ".error\n"
    "{\n"
    "	background: #d24040;\n"
    "	color: white;\n"
    "	border-color: #b74e4e;\n"
    "}\n"
    ".warning\n"
    "{\n"
    "	background: #e8d34e;\n"
    "	color: black;\n"
    "	border-color: #ca9466;\n"
    "}\n"
    ".info\n"
    "{\n"
    "	background: #2a9cdb;\n"
    "	color: white;\n"
    "	border-color: #3b70bb;\n"
    "}\n"
    ".debug\n"
    "{\n"
    "	background: #3fab2a;\n"
    "	color: white;\n"
    "	border-color: #226c13;\n"
    "}\n"
    "scrollbarvertical\n"
    "{\n"
    "	width: 16px;\n"
    "	scrollbar-margin: 16px;\n"
    "}\n"
    "scrollbarhorizontal\n"
    "{\n"
    "	height: 16px;\n"
    "	scrollbar-margin: 16px;\n"
    "}\n"
    "scrollbarvertical slidertrack,\n"
    "scrollbarhorizontal slidertrack\n"
    "{\n"
    "	background: #aaa;\n"
    "	border-color: #888;\n"
    "}\n"
    "scrollbarvertical slidertrack\n"
    "{\n"
    "	border-left-width: 1px;\n"
    "}\n"
    "scrollbarhorizontal slidertrack\n"
    "{\n"
    "	height: 15px;\n"
    "	border-top-width: 1px;\n"
    "}\n"
    "scrollbarvertical sliderbar,\n"
    "scrollbarhorizontal sliderbar\n"
    "{\n"
    "	background: #ddd;\n"
    "	border-color: #888;\n"
    "}"
    "scrollbarvertical sliderbar\n"
    "{\n"
    "	border-width: 1px 0px;\n"
    "	margin-left: 1px;\n"
    "}\n"
    "scrollbarhorizontal sliderbar\n"
    "{\n"
    "	height: 15px;\n"
    "	border-width: 0px 1px;\n"
    "	margin-top: 1px;\n"
    "}\n"
    "scrollbarcorner\n"
    "{\n"
    "	background: #888;\n"
    "}\n"
    "handle#size_handle\n"
    "{\n"
    "	position: absolute;\n"
    "	width: 16px;\n"
    "	height: 16px;\n"
    "	bottom: -2px;\n"
    "	right: 2px;\n"
    "	background-color: #888;\n"
    "}\n";
