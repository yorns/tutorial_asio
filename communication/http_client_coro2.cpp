//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, coroutine
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <functional>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what, const std::function<void()>& finish)
{
    std::cerr << what << ": " << ec.message() << "\n";
    finish();
}

// Performs an HTTP GET and prints the response
void
do_session(
    std::string const& host,
    std::string const& port,
    std::string const& target,
    int version,
    boost::asio::io_context& ioc,
    tcp::socket& socket,
    const std::function<void()>& finish,
    boost::asio::yield_context yield)
{
    boost::system::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver{ioc};

    // Look up the domain name
    auto const results = resolver.async_resolve(host, port, yield[ec]);
    if(ec)
        return fail(ec, "resolve", finish);

    // Make the connection on the IP address we get from a lookup
    boost::asio::async_connect(socket, results.begin(), results.end(), yield[ec]);
    if(ec)
        return fail(ec, "connect", finish);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::async_write(socket, req, yield[ec]);
    if(ec)
        return fail(ec, "write", finish);

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer b;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::async_read(socket, b, res, yield[ec]);
    if(ec)
        return fail(ec, "read", finish);

    // Write the message to standard out
    std::cout << res << std::endl;

    // Gracefully close the socket
    socket.shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != boost::system::errc::not_connected)
        return fail(ec, "shutdown", finish);

    // If we get here then the connection is closed gracefully
    finish();
}

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    // Check command line arguments.
    if(argc != 4 && argc != 5)
    {
        std::cerr <<
            "Usage: " << argv[0] << " <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
            "Example:\n" <<
            "    " << argv[0] << " www.example.com 80 /\n" <<
            "    " << argv[0] << " www.example.com 80 / 1.0\n";
        return EXIT_FAILURE;
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

    // The io_context is required for all I/O
    boost::asio::io_context ioc;
    tcp::socket socket{ioc};

    net::signal_set signals(ioc, SIGINT, SIGTERM);

    // Launch the asynchronous operation
    boost::asio::spawn(ioc, std::bind(
        &do_session,
        std::string(host),
        std::string(port),
        std::string(target),
        version,
        std::ref(ioc),
        std::ref(socket),
        [&signals](){signals.cancel();},
        std::placeholders::_1));


    // Capture SIGINT and SIGTERM to perform a clean shutdown
    signals.async_wait(
            [&socket](boost::system::error_code const&, int signal)
            {
                socket.close();
            });


    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();

    return EXIT_SUCCESS;
}
