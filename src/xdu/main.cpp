/*
 * main.cpp
 *
 *  Created on: Feb 24, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include <config.h>

#include "log.h"

int main( void )
{
	init_boost_log();

	BOOST_LOG_TRIVIAL( debug )  << "and this song is for any kid who gets picked on.";
	BOOST_LOG_TRIVIAL( info )  << "lemme know when u get ready.";
	BOOST_LOG_TRIVIAL( warning )  << "still got love for the streets.";
}
