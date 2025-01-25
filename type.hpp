#pragma once

#include <variant>
#include <memory>
#include <iostream>

namespace hm {

class TCon;
class TUniVar;
class TBoundVar;
class TApp;

class Type {
  using Ty = std::variant<TUniVar,TBoundVar,TCon,TApp>;
  std::shared_ptr<Ty> ref;

  friend TUniVar;
  Type(): ref{} {}
  bool not_null() const { return bool{ref}; }
  void* raw() const { return ref.get(); }

public:

  template <typename T>
  Type(T &&x): ref{std::make_shared<Ty>(std::in_place_type<T>,std::forward<T>(x))} {}
  
  enum Shape: size_t { IsTUniVar, IsTBoundVar, IsTCon, IsTApp };
  Shape shape() const;
  TUniVar& as_uni_var();
  TBoundVar& as_bound_var();
  TCon& as_con();
  TApp& as_app();

  /// The argument should be an unbound unification variable, `v`
  /// and different from this type.
  /// Checks that it is safe to unify this type with `v`. In particular:
  ///   * Checks that `v` does not occur in this type.
  ///   * It also checks that this type does not contain bound variables
  ///     exceeding the `v`'s bound variable limit.
  ///    * Also restricts the limits to unbound type variables to
  ///      match `v`'s limit.  Note that the limits might be modified even
  ///      if we end up returning `false`.
  bool fits_in(Type &v);
  
  bool unify(Type &other);
  void zonk();
  bool operator == (Type& other);
  bool ptr_eq(Type const& other) const { return raw() == other.raw(); } 

  std::ostream& dump(std::ostream& os, size_t indent);
};



struct TUniVar {
  Type forward;  // may be null
  
  int bound_var_limit;
  // Only variables that are less than this are in scope.

  TUniVar() = delete;
  TUniVar(int limit): forward{}, bound_var_limit{limit} {}
  TUniVar(TUniVar&&) = default;
  TUniVar(TUniVar const&) = delete;
  TUniVar& operator = (TUniVar const&) = delete;

  void reduce_scope_to(TUniVar& other) {
    if (bound_var_limit > other.bound_var_limit) {
      bound_var_limit = other.bound_var_limit;
    }
  }

  std::ostream& dump(std::ostream& os, size_t indent);
};


class TBoundVar {
  int index;
public:
  TBoundVar() = delete;
  TBoundVar(int index): index{index} {}
  bool operator == (TBoundVar const& other) const { return index == other.index; }
  bool operator != (TBoundVar const& other) const { return !(*this == other); }
  bool fits_in(TUniVar& x) const { return index < x.bound_var_limit; }
  std::ostream& dump(std::ostream& os, size_t indent);
};


class TCon {
  int name;
public:
  TCon(int name): name{name} {}
  bool operator == (TCon const& other) const { return name == other.name; }
  bool operator != (TCon const& other) const { return !(*this == other); }
  std::ostream& dump(std::ostream& os, size_t indent);
};


struct TApp {
  Type fun;
  Type arg;

  TApp() = delete;
  TApp(TApp const&) = delete;
  TApp(TApp &&) = default;
  TApp& operator = (TApp const&) = delete;

  template <typename F, typename X>
  TApp(F &&fun, X &&arg): fun{std::forward<F>(fun)}, arg{std::forward<X>(arg)} {}

  bool unify(TApp &other) {
    return fun.unify(other.fun) && arg.unify(other.arg);
  }

  bool operator == (TApp &other) {
    return fun == other.fun && arg == other.arg;
  }

  bool operator != (TApp &other) { return !(*this == other); } 

  bool fits_in(Type& v) {
    return fun.fits_in(v) && arg.fits_in(v); 
  }
  

  std::ostream& dump(std::ostream& os, size_t indent);
};


inline Type tuni_var(int limit) { return Type{TUniVar{limit}}; } 
inline Type tbound_var(int index) { return Type{TBoundVar{index}}; }
inline Type tcon(int name) { return Type{TCon{name}}; }
template <typename F, typename X>
inline Type tapp(F &&f, X &&x) { return Type{TApp{std::forward<F>(f),std::forward<X>(x)}}; }


inline Type::Shape Type::shape() const { return Shape{ref->index()}; }
inline TUniVar& Type::as_uni_var() { return std::get<TUniVar>(*ref); }
inline TBoundVar& Type::as_bound_var() { return std::get<TBoundVar>(*ref); }
inline TCon& Type::as_con() { return std::get<TCon>(*ref); }
inline TApp& Type::as_app() { return std::get<TApp>(*ref); }


inline void Type::zonk() {
  if (shape() == IsTUniVar) {
    auto& r = as_uni_var();
    if (r.forward.not_null()) {
      r.forward.zonk();
      ref = r.forward.ref;
    }
  }
}


}


