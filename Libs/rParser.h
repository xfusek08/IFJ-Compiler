/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    rParser.h
 * \brief   Main parser using recursive descent
 *
 * Parser provides main managing functionality of program.
 * It is getting list of tokens from lexical analyzer and based on specified grammar,
 * recursively checks syntactic and semantic validity and generates controlling code.
 * As is Definition of functions conditional jumps etc.
 * It calls supportive precedence paraser for evaluating and generating code from expressions.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    16.11.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _RParser
#define _RParser

/**
 * Main function of parser.
 *
 * The only function in the intervace of module.
 */
void rparser_processProgram();

#endif _RParser
