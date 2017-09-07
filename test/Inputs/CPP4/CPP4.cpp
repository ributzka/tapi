#define EXTERN_TEMPLATE(...)
#include "Templates.h"

namespace test2 {

template <> int &Foo<int>::instance() {
  static int a = 0;
  return a;
}

} // end namespace test2.
