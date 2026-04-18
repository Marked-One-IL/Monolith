#pragma once
#include <Parser/AST/Base.hpp>
#include <Helper/Types.hpp>
#include <vector>
#include <ostream>
#include <memory>

namespace Parser::AST
{
    struct Block
    {
        Block(std::vector<std::unique_ptr<const Parser::AST::Base>> new_commands);
        void print(std::ostream& stream, Uint depth) const;

        std::vector<std::unique_ptr<const Parser::AST::Base>> commands;
    };
}
