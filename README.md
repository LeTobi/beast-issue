[archived]
# beast-issue
Discovered on Boost Version 290. The websocket::stream doesn't seem to reset its internal state properly.
This is the shortest code I came up with to reproduce the error.

expected output:

```
Will it crash?... 
not yet!
Connection established
Connection finished: End of file
Will it crash?... 
not yet!
Connection established
Connection finished: Operation canceled
Will it crash?... 
socket-recycling: /usr/include/boost/beast/websocket/impl/stream_impl.hpp:192: void boost::beast::websocket::stream< <template-parameter-1-1>, <anonymous> >::impl_type::reset() [with NextLayer = boost::asio::basic_stream_socket<boost::asio::ip::tcp>; bool deflateSupported = true]: Assertion `status_ != status::open' failed.
```
# edit
websocket::stream is not intended to be reused. In-place reconstruction of the object does seem to work however.
