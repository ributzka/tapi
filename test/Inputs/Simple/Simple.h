#import <Foundation/Foundation.h>

// Useless forward declaration. This is used for testing.
@class FooBar;
@protocol FooProtocol;


// Test public global.
extern int publicGlobalVariable;

// Test weak public global.
extern int weakPublicGlobalVariable __attribute__((weak));

// Test public ObjC class
@interface Simple : NSObject
@end

__attribute__((objc_exception))
@interface Base : NSObject
@end

@interface SubClass : Base
@end

NS_AVAILABLE(10_11, 9_0)
@protocol FooProtocol
@end
