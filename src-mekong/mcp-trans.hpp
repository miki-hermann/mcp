/**************************************************************************
 *                                                                        *
 *                                                                        *
 *        Multiple Characterization Problem (MCP)                         *
 *                                                                        *
 * Author:   Miki Hermann                                                 *
 * e-mail:   hermann@lix.polytechnique.fr                                 *
 * Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France             *
 *                                                                        *
 * Author:   Gernot Salzer                                                *
 * e-mail:   gernot.salzer@tuwien.ac.at                                   *
 * Address:  Technische Universitaet Wien, Vienna, Austria                *
 *                                                                        *
 * Author:   César Sagaert                                                *
 * e-mail:   cesar.sagaert@ensta-paris.fr                                 *
 * Address:  ENSTA Paris, Palaiseau, France                               *
 *                                                                        *
 * Version: all                                                           *
 *     File:    src-mekong/mcp-matrix+formula.hpp                         *
 *                                                                        *
 *      Copyright (c) 2019 - 2025                                         *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <unordered_map>

using namespace std;

enum Index : bool {LOCAL = 0, GLOBAL = 1};
enum Token : char {
  ERROR,
  // used symbols ... must agree with symbol_tab in mcp-trans.cpp
  EQUAL,	// =
  COLON,	// :
  SCOL,		// ;
  LBRA,		// [
  RBRA,		// ]
  PLUS,		// +
  MINUS,	// -
  QMARK,	// ?
  DOLLAR,	// $
  CARET,	// ^
  LPAR,		// (
  RPAR,		// )
  // limit of used symbols
  // unused symbols
  EXMARK,	// !
  AT,		// @
  PERCENT,	// %
  AND,		// &
  OR,		// |
  STAR,		// *
  LESS,		// <
  GREATER,	// >
  COMMA,	// ,
  SLASH,	// /
  TILDA,	// ~
  DOT,		// .
  USCORE,	// _
  BSLASH,	// \ //
  // limit of unused symbols
  // entities
  STRING,
  NUM,
  FLOAT,
  SCIENTIFIC,
  // reserved words
  CONCEPT,
  PIVOT,
  BOOL,
  ENUM,
  UP,
  DOWN,
  INT,
  DISJOINT,
  OVERLAP,
  SPAN,
  WARP,
  CHECKPOINTS,
  INF,			// minimum or maximum infinity
  // not implemented yet
  STEP,
  DATE,
  DNUM,
  DVAL,
  TIME,
  TNUM,
  TVAL,
  WEEK,
  MONTH,
  YEAR,
  JAN,
  FEB,
  MAR,
  APR,
  MAY,
  JUN,
  JUL,
  AUG,
  SEP,
  OCT,
  NOV,
  DEC,
  MON,
  TUE,
  WED,
  THU,
  FRI,
  SAT,
  SUN
};
enum Token_Type : char {
  GENERAL_T = 0,
  DATE_T    = 1,
  TIME_T    = 2,
  WEEK_T    = 3,
  MONTH_T   = 4,
  YEAR_T    = 5
};

const unordered_map<string, Token> keywords = {
  {"concept", CONCEPT},
  {"pivot", PIVOT},
  {"bool", BOOL},
  {"enum", ENUM},
  {"up", UP},
  {"down", DOWN},
  {"int", INT},
  {"dj", DISJOINT},
  {"disjoint", DISJOINT},
  {"over", OVERLAP},
  {"overlap", OVERLAP},
  {"span", SPAN},
  {"warp", WARP},
  {"cp", CHECKPOINTS},
  {"checkpoints", CHECKPOINTS},
  {"inf", INF},
  {"INF", INF},
  // not implemented yet
  {"step", STEP},
  {"date", DATE},
  {"time", TIME},
  {"week", WEEK},
  {"month", MONTH},
  {"year", YEAR}
};
const unordered_map<Token,string> token_string = {
  {ERROR,	"ERROR"},
  //symbols
  {EQUAL,	"EQUAL"},
  {COLON,	"COLON"},
  {SCOL,	"SCOL"},
  {LPAR,	"LPAR"},
  {RPAR,	"RPAR"},
  {LBRA,	"LBRA"},
  {RBRA,	"RBRA"},
  {PLUS,	"PLUS"},
  {MINUS,	"MINUS"},
  {QMARK,	"QMARK"},
  {EXMARK,	"EXMARK"},
  {AT,		"AT"},
  {DOLLAR,	"DOLLAR"},
  {PERCENT,	"PERCENT"},
  {AND,		"AND"},
  {OR,		"OR"},
  {STAR,	"STAR"},
  {USCORE,	"USCORE"},
  {LESS,	"LESS"},
  {GREATER,	"GREATER"},
  {COMMA,	"COMMA"},
  {DOT,		"DOT"},
  {SLASH,	"SLASH"},
  {BSLASH,	"BSLASH"},
  {TILDA,	"TILDA"},
  {CARET,	"CARET"},
  //entities
  {STRING,	"STRING"},
  {NUM,		"NUM"},
  {FLOAT,	"FLOAT"},
  {SCIENTIFIC,	"SCIENTIFIC"},
  // reserved words
  {CONCEPT,	"CONCEPT"},
  {PIVOT,	"PIVOT"},
  {BOOL,	"BOOL"},
  {ENUM,	"ENUM"},
  {UP,		"UP"},
  {DOWN,	"DOWN"},
  {INT,		"INT"},
  {DISJOINT,	"DISJOINT"},
  {OVERLAP,	"OVERLAP"},
  {SPAN,	"SPAN"},
  {WARP,	"WARP"},
  {CHECKPOINTS,	"CHECKPOINTS"},
  {INF,         "INF"},
  // not implemented yet
  {STEP,	"STEP"},
  {DATE,	"DATE"},
  {DNUM,	"DNUM"},
  {DVAL,	"DVAL"},
  {TIME,	"TIME"},
  {TNUM,	"TNUM"},
  {TVAL,	"TVAL"},
  {WEEK,	"WEEK"},
  {MONTH,	"MONTH"},
  {YEAR,	"YEAR"},
  {JAN,		"JAN"},
  {FEB,		"FEB"},
  {MAR,		"MAR"},
  {APR,		"APR"},
  {MAY,		"MAY"},
  {JUN,		"JUN"},
  {JUL,		"JUL"},
  {AUG,		"AUG"},
  {SEP,		"SEP"},
  {OCT,		"OCT"},
  {NOV,		"NOV"},
  {DEC,		"DEC"},
  {MON,		"MON"},
  {TUE,		"TUE"},
  {WED,		"WED"},
  {THU,		"THU"},
  {FRI,		"FRI"},
  {SAT,		"SAT"},
  {SUN,		"SUN"}
};
const unordered_map<string,Token> reverse_string = {
  {"BOOL",	BOOL},
  {"ENUM",	ENUM},
  {"UP",	UP},
  {"DOWN",	DOWN},
  {"INT",	INT},
  {"DISJOINT",	DISJOINT},
  {"OVERLAP",	OVERLAP},
  {"SPAN",	SPAN},
  {"WARP",	WARP},
  {"CHECKPOINTS",	CHECKPOINTS},
  {"INF",       INF}
  // not implemented yet
  // {"STEP",	STEP},
  // {"DATE",	DATE},
  // {"DNUM",	DNUM},
  // {"DVAL",	DVAL},
  // {"TIME",	TIME},
  // {"TNUM",	TNUM},
  // {"TVAL",	TVAL},
  // {"WEEK",	WEEK},
  // {"MONTH",	MONTH},
  // {"YEAR",	YEAR},
  // {"JAN",	JAN},
  // {"FEB",	FEB},
  // {"MAR",	MAR},
  // {"APR",	APR},
  // {"MAY",	MAY},
  // {"JUN",	JUN},
  // {"JUL",	JUL},
  // {"AUG",	AUG},
  // {"SEP",	SEP},
  // {"OCT",	OCT},
  // {"NOV",	NOV},
  // {"DEC",	DEC},
  // {"MON",	MON},
  // {"TUE",	TUE},
  // {"WED",	WED},
  // {"THU",	THU},
  // {"FRI",	FRI},
  // {"SAT",	SAT},
  // {"SUN",	SUN}
};
