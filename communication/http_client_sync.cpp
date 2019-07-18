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
// Example: HTTP client, synchronous
//
//------------------------------------------------------------------------------

/* example calls
 ./httpRestClient echo.jsontest.com 80 /title/ipsum/content/blah
 ./httpRestClient headers.jsontest.com 80 /
 ./httpRestClient time.jsontest.com 80 /
 */


//[example_http_client


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 3 && argc != 4)
        {
            std::cerr <<
                "Usage: http-client-sync <host> <port> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    " << argv[0] << " www.example.com 80\n" <<
                "    " << argv[0] << " www.example.com 80 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        int version = argc == 4 && !std::strcmp("1.0", argv[3]) ? 10 : 11;

        // The io_context is required for all I/O
        boost::asio::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        tcp::socket socket{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(socket, results.begin(), results.end());

        std::string target;

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        while (true) {
            std::cout << "target: ";
            std::cin >> target;

            if (target == "end")
                break;

            // Set up an HTTP GET request message
            http::request<http::empty_body> req{http::verb::get, target, version};
            req.set(http::field::host, host);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.keep_alive(true);

            // Send the HTTP request to the remote host
            http::write(socket, req);

            // Declare a container to hold the response
            //http::response<http::dynamic_body> res;
            http::response<http::string_body> res;

            // Receive the HTTP response
            http::read(socket, buffer, res);

            // Write the message to standard out
//            std::cout << "Full message returned:\n" << res << std::endl;
//            std::cout << "Only body content:\n" << boost::beast::buffers_to_string(res.body().data()) << std::endl;
            std::cout << "Only body content:\n" << res.body() << std::endl;

        }
        // Gracefully close the socket
        boost::system::error_code ec;
        socket.shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != boost::system::errc::not_connected)
            throw boost::system::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//]
