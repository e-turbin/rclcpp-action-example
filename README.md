# rclcpp-action-example

A reference implementation of common rclcpp_action nodes - client and server, as well as the interface used for communication inbetween. 
Focuses on code readability and stricter memory management (via `const auto&`).

## Key Features
* **Modern C++**: Reject `std::bind` and `std::placeholders`, embrace lambda.
* **Type safety**: Strict use of `const auto&` in lambda to prevent copying.
* **Readable formatting**: Formatted with care.
