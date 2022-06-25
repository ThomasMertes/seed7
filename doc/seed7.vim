"----------------------------------------------------------------------------
"  Description: Vim sd7 syntax file
"     Language: Seed7
"	   $Id: s7.vim 001 2021-12-05 22:55:01Z jg $
"    Copyright: Copyright (C) 2021 J. Gritsch
"   Maintainer: Johannes Gritsch
"      $Author: jg $
"	 $Date: 2021-12-05 22:55:01 +0200 (Sa, 5. Dez 2021) $
"      Version: 1.01
"    $Revision: 001 $
"      History: 05.12.2021 JG created
"               10.12.2021 JG removed case insensitivity

"------------------------------------------------------------------------------
" The formal spec of Seed7 "Seed7 Reference Manual".
" For more Seed7 info, see http://seed7.sourceforge.net/
"
" This vim syntax file works on vim 7.0 and above
"------------------------------------------------------------------------------

if exists("b:current_syntax") || version < 700
    finish
endif

let b:current_syntax = "sd7"

" Section: Seed7 is entirely case-sensitive. {{{1
"
" Section: Highlighting commands {{{1
"
" There are some reserved words in total in sd7. Some keywords are used in more than one way. For example:
" 1. "end" is a general keyword, but "end if" ends a Conditional.
"
" Section: Operators {{{1
"
syntax keyword  sd7Operator and conv digits div exp in lpad lapd0 mdiv mod mult not or parse rem rpad sci times varConv
syntax match    sd7Operator "[-+*/<>&[\]:{}@]"
syntax keyword  sd7Operator ** ><
syntax match    sd7Operator "[/<>]="
syntax match    sd7Operator "\.\."
syntax match    sd7Operator "="

" Section: := {{{1
"
syntax match sd7Assignment		"(\+|-|\*|/|<<|>>|&|><|\||@)?:="

" Section: Numbers, including floating point, exponents, and alternate bases. {{{1
"
syntax match   sd7Number		"\<\d\d*\(_\|#[0-9a-zA-Z][0-9a-zA-Z]*_\{0,1\}\|[Ee]\(+\)\{0,1\}\d\d*\|\.\d\d*\([Ee]\(+\|-\)\{0,1\}\d\d*\)\{0,1\}\)\{0,1\}\>"

" Section: Boolean Constants {{{1
" Boolean Constants.
syntax keyword sd7Boolean	TRUE FALSE

" Section: Warn C/C++ {{{1
" Warn people who try to use C/C++ notation erroneously:
"
syntax match sd7Error "//"
syntax match sd7Error "/\*"
syntax match sd7Error "\*/"
syntax match sd7Error "=="
syntax match sd7Error "\<0x[0-9a-zA-Z][0-9a-zA-Z]*\>"
syntax match sd7Error "\<\d\d*[Ee]-\d\d*\>"
syntax match sd7Error "\(''\|'[^\\].'\|'\\\d\d*'\)"
syntax match sd7Error "\.\d\d*"
syntax match sd7Error "\d\d*\.[^0-9]"

" Section: end {{{1
syntax region   sd7Inc contains=sd7String start="\<include\>" end="$"

syntax keyword  sd7Preproc		 pragma proc type array func hash set varfunc end

syntax keyword  sd7Repeat	 exit for loop reverse while repeat until
syntax match    sd7Repeat		   "\<end\s\+loop\>"

syntax keyword  sd7Statement is

syntax keyword sd7BuiltinSub	write writeln

" Section: Types
syntax keyword sd7BuiltinType	bigInteger bigRational bin32      bin64      bitset     boolean    char
syntax keyword sd7BuiltinType	clib_file  color       complex    duration   expr       file       float
syntax keyword sd7BuiltinType	integer    object      program    rational   reference
syntax keyword sd7BuiltinType	ref_list   string      text       time       type       void
syntax keyword sd7BuiltinType	PRIMITIVE_WINDOW

"Section: Conditionals {{{1
syntax match    sd7Conditional  "\<then\>"
syntax match    sd7Conditional	"\<else\>"
syntax match    sd7Conditional	"\<end\s\+if\>"
syntax match    sd7Conditional	"\<end\s\+case\>"
syntax match    sd7Conditional	"\<end\s\+select\>"
syntax keyword  sd7Conditional	if case otherwise
syntax keyword  sd7Conditional	elsif when

" Section: other keywords {{{1
syntax keyword  sd7Keyword	block do exception key new null out
syntax keyword  sd7Keyword	downto enum forward inout local of param range ref
syntax keyword  sd7Keyword	return struct sub syntax system to val

" Section: begin keywords {{{1
"
" These keywords begin various constructs, and you _might_ want to
" highlight them differently.
"
syntax keyword  sd7Begin	const var begin local

" Section: String and character constants. {{{1
"
syntax match   sd7Escapes       +\\\([abefnrtv\\'"A-Z]\|\d\d*\(;\|#[0-9a-zA-Z][0-9a-zA-Z]*;\)\)+
syntax match   sd7WrongEscapes  +\\\([cdghijklmopqsuwxyz!#%&()+,/_`]\)+
syntax region  sd7String        start=+"\|[ \t^]\\+ end=+"\|\\[ \t$]+ contains=@Spell,sd7Escapes,sd7WrongEscapes extend
syntax match   sd7Character     "'\([^\\]\|\\\([abefnrtv\\'"A-Z]\|\d\d*\(;\|#[0-9a-zA-Z][0-9a-zA-Z]*;\)\)\)'"

" Section: Todo (only highlighted in comments) {{{1
"
syntax keyword sd7Todo contained TODO FIXME XXX NOTE

" Section: Comments {{{1
"
syntax region  sd7Comment contains=sd7Todo,sd7LineError,@Spell,sd7Comment start="(\*" end="\*)"
syntax region  sd7CommentL contains=sd7Todo,sd7LineError,@Spell start="#" end="$"

" Section: Var {{{1
" syntax region sd7var contains=sd7String,sd7Number,sd7Character,sd7Statement start="\<var\>" end="\;"

" Section: The default methods for highlighting. Can be overridden later. {{{1
"
highlight def link sd7Character	    Character
highlight def link sd7Comment	    Comment
highlight def link sd7CommentL	    Comment
highlight def link sd7Conditional   Conditional
highlight def link sd7Keyword	    Keyword
highlight def link sd7Label	    Label
highlight def link sd7Number	    Number
highlight def link sd7Sign	    Number
highlight def link sd7Operator	    PreProc
highlight def link sd7Preproc	    PreProc
highlight def link sd7Repeat	    Repeat
highlight def link sd7Special	    Special
highlight def link sd7Begin	    Statement
highlight def link sd7Statement	    Statement
highlight def link sd7BuiltinSub    Type
highlight def link sd7Escapes	    String
highlight def link sd7WrongEscapes  Error
highlight def link sd7String	    String
highlight def link sd7Structure	    Structure
highlight def link sd7Todo	    Todo
highlight def link sd7Type	    Type
highlight def link sd7Typedef	    Typedef
highlight def link sd7StorageClass  StorageClass
highlight def link sd7Boolean	    Boolean
highlight def link sd7Exception	    Exception
highlight def link sd7Attribute	    Tag
highlight def link sd7var	    Tag
highlight def link sd7Inc	    Include
highlight def link sd7Error	    Error
highlight def link sd7SpaceError    Error
highlight def link sd7LineError	    Error
highlight def link sd7BuiltinType   Type
highlight def link sd7Assignment    Special

" Subsection: Begin, End {{{2
"

finish " 1}}}
