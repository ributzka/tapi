#ifndef TEMPLATES_H
#define TEMPLATES_H

#ifndef EXTERN_TEMPLATE
#define EXTERN_TEMPLATE(...) extern template __VA_ARGS__;
#endif

namespace test1 {

template <class A = int> class Foo {
public:
  static A &run() { return instance(); }
  static short getX();

private:
  Foo() = delete;
  static A &instance();
  static short x;
};

template <class A> short Foo<A>::x = 0;

template <class A> inline short Foo<A>::getX() { return x; }

} // end namespace test1.

namespace test2 {

template <class A = int> class Foo {
public:
  static A &run() { return instance(); }
  static short getX();

private:
  Foo() = delete;
  static A &instance();
  static short x;
};

template <class A> short Foo<A>::x = 0;

template <class A> inline short Foo<A>::getX() { return x; }

EXTERN_TEMPLATE(int &Foo<int>::instance())

} // end namespace test2.

#endif
