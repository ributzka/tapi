#ifndef VTABLE_H
#define VTABLE_H

// Simple test class with no virtual functions. There should be no vtable or
// RTTI.
namespace test1 {
class Simple {
public:
  void run();
};
} // end namespace test1

// Simple test class with virtual function. There should be an external vtable
// and RTTI.
namespace test2 {
class Simple {
public:
  virtual void run();
};
} // end namespace test2

// Abstract class with no sub classes. There should be no vtable or RTTI.
namespace test3 {
class Abstract {
public:
  virtual ~Abstract() {}
  virtual void run() = 0;
};
} // end namespace test3

// Abstract base class with a sub class. There should be weak-def RTTI for the
// abstract base class.
// The sub-class should have vtable and RTTI.
namespace test4 {
class Base {
public:
  virtual ~Base() {}
  virtual void run() = 0;
};

class Sub : public Base {
public:
  void run() override;
};
} // end namespace test4

// Abstract base class with a sub class. Same as above, but with a user defined
// inlined destructor.
namespace test5 {
class Base {
public:
  virtual ~Base() {}
  virtual void run() = 0;
};

class Sub : public Base {
public:
  virtual ~Sub() {}
  void run() override;
};
} // end namespace test5

// Abstract base class with a sub class. Same as above, but with a different
// inlined key method.
namespace test6 {
class Base {
public:
  virtual ~Base() {}
  virtual void run() = 0;
};

class Sub : public Base {
public:
  virtual void foo() {}
  void run() override;
};
} // end namespace test6

// Abstract base class with a sub class. Overloaded method is implemented
// inline. No vtable or RTTI.
namespace test7 {
class Base {
public:
  virtual ~Base() {}
  virtual bool run() = 0;
};

class Sub : public Base {
public:
  bool run() override { return true; }
};
} // end namespace test7

// Abstract base class with a sub class. Overloaded method has no inline
// attribute and is recognized as key method,
// but is later implemented inline. Weak-def RTTI only.
namespace test8 {
class Base {
public:
  virtual ~Base() {}
  virtual void run() = 0;
};

class Sub : public Base {
public:
  void run() override;
};

inline void Sub::run() {}
} // end namespace test8

namespace test9 {
class Base {
public:
  virtual ~Base() {}
  virtual void run1() = 0;
  virtual void run2() = 0;
};

class Sub : public Base {
public:
  void run1() override {}
  void run2() override;
};

inline void Sub::run2() {}
} // end namespace test9

namespace test10 {
class Base {
public:
  virtual ~Base() {}
  virtual void run1() = 0;
  virtual void run2() = 0;
};

class Sub : public Base {
public:
  void run1() override {}
  inline void run2() override;
};

void Sub::run2() {}
} // end namespace test10

namespace test11 {
class Base {
public:
  virtual ~Base() {}
  virtual void run1() = 0;
  virtual void run2() = 0;
  virtual void run3() = 0;
};

class Sub : public Base {
public:
  void run1() override {}
  void run2() override;
  void run3() override;
};

inline void Sub::run2() {}
} // end namespace test11

namespace test12 {
template <class T> class Simple {
public:
  virtual void foo() {}
};
extern template class Simple<int>;
} // end namespace test12

#endif
