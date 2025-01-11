#pragma once

#include <variant>
#include <memory>
#include <iostream>

namespace hm {

class TCon;
class TVar;
class TApp;

class Type {
  using Ty = std::variant<TVar,TCon,TApp>;
  std::shared_ptr<Ty> ref;

  friend TVar;
  Type(): ref{} {}
  bool not_null() const { return bool{ref}; }
  void* raw() const { return ref.get(); }

public:

  template <typename T>
  Type(T &&x): ref{std::make_shared<Ty>(std::in_place_type<T>,std::forward<T>(x))} {}
  
  enum Shape: size_t { IsTVar, IsTCon, IsTApp };
  Shape shape() const;
  TVar& as_var();
  TCon& as_con();
  TApp& as_app();

  bool occurs(Type &v);
  bool unify(Type &other);
  void zonk();
  bool operator == (Type& other);
  bool ptr_eq(Type const& other) const { return raw() == other.raw(); } 

  std::ostream& dump(std::ostream& os, size_t indent);
};



struct TVar {
  Type forward;  // may be null

  TVar(): forward{} {}
  TVar(TVar&&) = default;
  TVar(TVar const&) = delete;
  TVar& operator = (TVar const&) = delete;

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

  bool occurs(Type& v) {
    return fun.occurs(v) || arg.occurs(v); 
  }
  

  std::ostream& dump(std::ostream& os, size_t indent);
};


inline Type tvar() { return Type{TVar{}}; } 
inline Type tcon(int name) { return Type{TCon{name}}; }
template <typename F, typename X>
inline Type tapp(F &&f, X &&x) { return Type{TApp{std::forward<F>(f),std::forward<X>(x)}}; }

inline Type::Shape Type::shape() const { return Shape{ref->index()}; }
inline TVar& Type::as_var() { return std::get<TVar>(*ref); }
inline TCon& Type::as_con() { return std::get<TCon>(*ref); }
inline TApp& Type::as_app() { return std::get<TApp>(*ref); }



inline void Type::zonk() {
  if (shape() == IsTVar) {
    auto& r = as_var();
    if (r.forward.not_null()) {
      r.forward.zonk();
      ref = r.forward.ref;
    }
  }
}


}


