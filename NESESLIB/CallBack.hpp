#pragma once
#include<functional>


namespace NESES
{   
// Callback class with variadic arguments
template <typename... Args>
class CallBack {
public:
    // Typedef for the callback function type
    using CallbackFunction = std::function<void(Args...)>;

    // Constructor
    CallBack() = default;

    // Set the callback function, taking it by const reference
    void setCallback(const CallbackFunction& cb) {
        callback = cb;
    }

    // Alternatively, use an rvalue reference to move the callback function
    void setCallback(CallbackFunction&& cb) {
        callback = std::move(cb);
    }
    // Invoke the callback function
    void invoke(Args... args) {
        if (callback) {
            callback(std::forward<Args>(args)...);
        }
    }

private:
    // Member to store the callback function
    CallbackFunction callback;
};
} // namespace NESES



//// Example functions to be used as callbacks
//void exampleFunction(int x, const std::string& y) {
//    std::cout << "exampleFunction called with: " << x << ", " << y << std::endl;
//}
//
// // Example class with a member function to be used as a callback
//class MyClass {
//public:
//    void memberFunction(int x, const std::string& y) {
//        std::cout << "MyClass::memberFunction called with: " << x << ", " << y << std::endl;
//    }
//};

// 
// 
//int main() {
//    // Create an instance of the Callback class
//    Callback<int, std::string> cb;
//
//    // Set the callback to a regular function
//    cb.setCallback([](int x, const std::string& y) {
//        std::cout << "Lambda called with: " << x << ", " << y << std::endl;
//        });
//
//    // Invoke the callback
//    cb.invoke(7, "Test");
//
//    // Set the callback to a functor
//    struct Functor {
//        void operator()(int x, const std::string& y) const {
//            std::cout << "Functor called with: " << x << ", " << y << std::endl;
//        }
//    } functor;
//
//    cb.setCallback(functor);
//
//    // Invoke the callback
//    cb.invoke(3, "Functor");
//
//    // Example of setting the callback to a member function
//    MyClass myObject;
//
//    // Using std::bind
//    cb.setCallback(std::bind(&MyClass::memberFunction, &myObject, std::placeholders::_1, std::placeholders::_2));
//    cb.invoke(42, "Using std::bind");
//
//    // Using a lambda
//    cb.setCallback([&myObject](int x, const std::string& y) {
//        myObject.memberFunction(x, y);
//        });
//    cb.invoke(99, "Using lambda");
//
//    return 0;
//}