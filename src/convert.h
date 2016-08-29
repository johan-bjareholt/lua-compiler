#pragma once

#include "ir.h"
#include <string>

void outMainBlock(std::stringstream& ss, BBlock& block);

std::string outBlock(BBlock& block, std::stringstream& ss, std::list<std::string>&);

void convertThreeAd(ThreeAd& op, std::stringstream& ss, std::list<std::string>&);
