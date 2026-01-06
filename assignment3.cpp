#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

/* =====================================================
   Semantic Error Handling
   ===================================================== */
class SemanticError : public runtime_error {
public:
    SemanticError(const string& msg)
        : runtime_error("Semantic Error: " + msg) {}
};

/* =====================================================
   Type System
   ===================================================== */
class Type {
public:
    string name;

    Type(const string& n) : name(n) {}

    virtual ~Type() {}

    virtual bool equals(Type* other) {
        return name == other->name;
    }
};

/* =====================================================
   Union Type
   ===================================================== */
class UnionType : public Type {
public:
    vector<Type*> memberTypes;

    UnionType(const vector<Type*>& types)
        : Type("union"), memberTypes(types) {}

    bool contains(Type* t) {
        for (auto mt : memberTypes) {
            if (mt->equals(t))
                return true;
        }
        return false;
    }
};

/* =====================================================
   Symbol Table Entry
   ===================================================== */
class Symbol {
public:
    string name;
    Type* type;

    Symbol(const string& n, Type* t) : name(n), type(t) {}
};

/* =====================================================
   Tagged Runtime Value (for correctness)
   ===================================================== */
class TaggedValue {
public:
    Type* typeTag;

    TaggedValue(Type* t) : typeTag(t) {}
};

/* =====================================================
   Helper: Field Lookup
   ===================================================== */
bool hasField(Type* type, const string& field) {
    // Example object type
    if (type->name == "Point") {
        return (field == "x" || field == "y");
    }
    return false;
}

/* =====================================================
   Assignment Type Checking
   ===================================================== */
void checkAssignment(Symbol* var, Type* exprType) {
    if (auto ut = dynamic_cast<UnionType*>(var->type)) {
        if (!ut->contains(exprType)) {
            throw SemanticError(
                "Cannot assign type '" + exprType->name +
                "' to union variable '" + var->name + "'"
            );
        }
    } else if (!var->type->equals(exprType)) {
        throw SemanticError(
            "Type mismatch in assignment to '" + var->name + "'"
        );
    }
}

/* =====================================================
   Type Discrimination (is-check)
   ===================================================== */
Type* checkIs(Type* exprType, Type* targetType) {
    if (auto ut = dynamic_cast<UnionType*>(exprType)) {
        if (!ut->contains(targetType)) {
            throw SemanticError(
                "Invalid type test: '" + targetType->name +
                "' not part of union"
            );
        }
        // Narrowed type inside conditional block
        return targetType;
    }

    if (!exprType->equals(targetType)) {
        throw SemanticError("Invalid type check");
    }

    return targetType;
}

/* =====================================================
   Safe Field Access Checking
   ===================================================== */
void checkFieldAccess(Type* exprType, const string& field) {
    if (dynamic_cast<UnionType*>(exprType)) {
        throw SemanticError(
            "Unsafe field access '" + field +
            "' on union type. Type discrimination required."
        );
    }

    if (!hasField(exprType, field)) {
        throw SemanticError(
            "Type '" + exprType->name +
            "' has no field '" + field + "'"
        );
    }
}
/* =====================================================
   Main: Demonstration
   ===================================================== */
int main() {
    try {
        // Primitive and object types
        Type* IntType    = new Type("int");
        Type* StringType = new Type("string");
        Type* PointType  = new Type("Point");

        // Union type: int | string | Point
        UnionType* U = new UnionType({IntType, StringType, PointType});

        // Symbol declaration
        Symbol* x = new Symbol("x", U);

        // ----- Valid assignment -----
        checkAssignment(x, IntType);

        // ----- Type discrimination -----
        Type* narrowedType = checkIs(x->type, PointType);

        // ----- Safe field access after discrimination -----
        checkFieldAccess(narrowedType, "x");

        cout << "Program is semantically correct." << endl;
    }
    catch (const SemanticError& e) {
        cout << e.what() << endl;
    }

    return 0;
}