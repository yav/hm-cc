#include "type.hpp"


namespace hm {



std::ostream& TVar::dump(std::ostream &os, size_t amt) {
  os << "tvar";
  if (forward.not_null()) os << " -> " << forward.raw();
  os << std::endl;
  return os;
}

std::ostream& TCon::dump(std::ostream &os, size_t amt) {
  os << "tcon " << name << std::endl;
  return os;
}

std::ostream& TApp::dump(std::ostream &os, size_t amt) {
  os << "tapp\n";
  fun.dump(os,amt+2);
  arg.dump(os,amt+2);
  return os;
}

std::ostream& Type::dump(std::ostream &os, size_t amt) {
  for(; amt > 0; --amt) os << ' ';
  os << "[" << this << "] ";
  switch (shape()) {
    case IsTVar: return as_var().dump(os,amt);
    case IsTCon: return as_con().dump(os,amt);
    case IsTApp: return as_app().dump(os,amt);
  }
  return os;
}

bool Type::occurs(Type& v) {
  zonk();
  if (ptr_eq(v)) return true;
  if (shape() == IsTApp) return as_app().occurs(v);
  return false;
}

static inline
bool bind(Type &v, Type &t) {
  if (v.ptr_eq(t)) return true;
  if (t.occurs(v)) return false;
  v.as_var().forward = t;
  return true;
}

bool Type::unify(Type &other) {
  zonk();
  other.zonk();
  if (ptr_eq(other)) return true;

  auto sh1 = shape();
  auto sh2 = other.shape();

  if (sh1 == IsTVar) {
    if (!bind(*this,other)) return false;
  } else 
  if (sh2 == IsTVar) {
    if (!bind(other,*this)) return false;
  } else
  if (sh1 != sh2) {
    return false;
  } else
  switch (sh1) {
    case IsTCon: if (as_con() != other.as_con()) return false; break;
    case IsTApp: if (!as_app().unify(other.as_app())) return false; break;
    default: return false; // unreachable
  }
  ref = other.ref;
  return true;
}

bool Type::operator == (Type &other) {
  zonk();
  other.zonk();
  if (ptr_eq(other)) return true;  // For TVar and shortcut
  auto sh1 = shape();
  auto sh2 = other.shape();
  if (sh1 != sh2) return false;
  switch (sh1) {
    case IsTCon: return as_con() == other.as_con();
    case IsTApp: return as_app() == other.as_app();
    default: return false; // TVars are equal by pointer
  }
}

}
