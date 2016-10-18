#pragma once

#include "ir.h"
#include <string>

void outMainBlock(std::ostream& ss, BBlock& block);

std::string outBlock(BBlock& block, std::ostream& ss, std::set<std::string>&, std::set<std::string>&, bool exit, bool localScope);

void outFunctionBlock(std::string funcname, BBlock& block, std::ostream& ss, std::set<std::string>& outputSymbols, std::set<std::string>& inputSymbols);

void convertThreeAd(ThreeAd& op, std::ostream& ss, std::set<std::string>&, std::set<std::string>&, bool localScope);
