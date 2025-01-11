#include "type.hpp"



int main(int argc, char* argv[]) {
  

  auto t1 = hm::tapp(hm::tvar(), hm::tvar());
  auto t2 = hm::tvar();

  auto dbg = [&t1,&t2] {
    std::cout << "t1 = ";
    t1.dump(std::cout,0);
    std::cout << "t2 = ";
    t2.dump(std::cout,0);
  };

  dbg();
  std::cout << "unify: " << t1.unify(t2) << "\n";
  dbg();
  std::cout << "zonk\n";
  t1.zonk();
  t2.zonk();
  dbg();

  return 0;
}


