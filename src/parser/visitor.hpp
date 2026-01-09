#pragma once
#include "ast.hpp"
#include "statements.hpp"

namespace parser {
class ASTVisitor {
public:
    virtual void visit(const LookAtStmt& stmt);
};
}
