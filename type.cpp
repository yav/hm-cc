#include "type.hpp"


namespace hm {



std::ostream& TUniVar::dump(std::ostream &os, size_t amt) {
  os << "tuni_var";
  if (forward.not_null()) os << " -> " << forward.raw();
  os << std::endl;
  return os;
}


std::ostream& TBoundVar::dump(std::ostream &os, size_t amt) {
  os << "tbound_var " << index << std::endl;
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
    case IsTUniVar: return as_uni_var().dump(os,amt);
    case IsTCon: return as_con().dump(os,amt);
    case IsTApp: return as_app().dump(os,amt);
  }
  return os;
}

bool Type::fits_in(Type& v) {
  zonk();
  if (ptr_eq(v)) return false;
  auto sh = shape();
  switch(sh) {
    case IsTApp: return as_app().fits_in(v);
    case IsTBoundVar: return as_bound_var().fits_in(v.as_uni_var());
    case IsTUniVar: v.as_uni_var().reduce_scope_to(v.as_uni_var()); return true;
    default: return true;
  }
}

static inline
bool bind(Type &v, Type &t) {
  if (v.ptr_eq(t)) return true;
  if (!t.fits_in(v)) return false;
  v.as_uni_var().forward = t;
  v = t;
  return true;
}

bool Type::unify(Type &other) {
  zonk();
  other.zonk();
  if (ptr_eq(other)) return true;

  auto sh1 = shape();
  auto sh2 = other.shape();

  if (sh1 == IsTUniVar) {
    if (!bind(*this,other)) return false;
  } else 
  if (sh2 == IsTUniVar) {
    if (!bind(other,*this)) return false;
  } else
  if (sh1 != sh2) {
    return false;
  } else
  switch (sh1) {
    case IsTBoundVar: if (as_bound_var() != other.as_bound_var()) return false; break;
    // XXX: delay for external reasoning

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
    case IsTBoundVar: if (as_bound_var() != other.as_bound_var()) return false; break;
    case IsTCon: if (as_con() != other.as_con()) return false; break;
    case IsTApp: if (as_app() != other.as_app()) return false; break;
    default: return false; // UniVars are equal by pointer
  }

  ref = other.ref;
  return true;
}

}
