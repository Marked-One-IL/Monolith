#include <Parser/AST/Block.hpp>

Parser::AST::Block::Block(std::vector<std::unique_ptr<const Parser::AST::Base>> new_commands) :
    commands(std::move(new_commands))
{
}

void Parser::AST::Block::print(std::ostream& stream, Uint depth) const
{
    if (this == nullptr) return;

    for (auto& ast : this->commands)
    {
        if (not ast) continue;
        ast->print(stream, depth + 1);
    }
    stream << '\n';
}
