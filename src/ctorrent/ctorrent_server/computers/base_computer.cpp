/*
 * base_computer.cpp
 *
 *  Created on: Jun 7, 2018
 *      Author: sergs
 */

#include "config.h"

#include "base_computer.h"

#include "./raw_src_computer/raw_src_computer.h"

#ifdef X86_64_BUILD
std::shared_mutex base_computer::mtx;
#endif
std::map<comp_type, std::shared_ptr<base_computer>> base_computer::registered_computers;

void base_computer::register_computers()
{
  register_computer<raw_src_computer>( comp_type::RAW_SRC );
}
