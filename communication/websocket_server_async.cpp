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
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <unordered_map>
#include <array>
#include <boost/asio/ip/tcp.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <regex>
#include <boost/lexical_cast.hpp>
#include "terminalColor.h"

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(boost::system::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class Lights : public std::enable_shared_from_this<Lights> {

public:
    enum class Color {
        red,
        blue,
        white,
        off
    };

private:
    std::array<Color,3> m_light {{Color::off, Color::off, Color::off }};

    std::map<Color, std::string> color {{Color::red, KRED}, {Color::blue, KBLU}, {Color::white, KWHT}, {Color::off, KWHT} };
    std::map<std::string, Color> colorName {{"red", Color::red}, {"blue", Color::blue}, {"white", Color::white}, {"off", Color::off} };

    uint32_t counter{0};

    void print() {
        std::cout << "\r " << counter << "  -  ";
        for(auto& i : m_light) { std::cout << color[i] << (i==Color::off?"         ":" [LIGHT] ") << RST << std::flush; }
        std::cout << "           ";
    }

public:
    Lights() {
    }

    bool changeColor(uint32_t id, const std::string& colorString) {
        if (colorName.find(colorString) != colorName.end()) {
            changeColor(id, colorName[colorString]);
            return true;
        }
        return false;
    }

        bool changeColor(uint32_t id, Color col) {
        if (id < m_light.size()) {
            m_light[id] = col;
            print();
            return true;
        }
        return false;
    }

    bool setCounter(uint32_t cnt) {
        counter = cnt;
        print();
        return true;
    }

};

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<tcp::socket> ws_;
    boost::asio::strand<
        boost::asio::io_context::executor_type> strand_;
    boost::beast::multi_buffer buffer_;
    std::shared_ptr<Lights> lights_;

public:
    // Take ownership of the socket
    explicit
    session(tcp::socket socket, std::shared_ptr<Lights> lights)
        : ws_(std::move(socket))
        , strand_(ws_.get_executor())
        , lights_(lights)
    {
    }

    // Start the asynchronous operation
    void
    run()
    {
        // Accept the websocket handshake
        ws_.async_accept(
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_accept,
                    shared_from_this(),
                    std::placeholders::_1)));
    }

    void
    on_accept(boost::system::error_code ec)
    {
        if(ec)
            return fail(ec, "accept");

        // Read a message
        do_read();
    }

    void
    do_read()
    {
        // Read a message into our buffer
        ws_.async_read(
            buffer_,
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_read,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    on_read(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if(ec == websocket::error::closed)
            return;

        if(ec)
            fail(ec, "read");

        std::string command {boost::beast::buffers_to_string(buffer_.data())};

        uint32_t id;
        std::string colorString;
        uint32_t counter;

        std::regex re("set (\\d*) (.*)");
        std::smatch match;
        if (std::regex_search(command, match, re)) {
            try {
                id = boost::lexical_cast<decltype(id)>(match[1].str());
                colorString = match[2].str();
                lights_->changeColor(id, colorString);
            }
            catch(boost::bad_lexical_cast& ex) {
                std::cerr << "cannot read request\n";
            }
        }

        std::regex re_count("counter (\\d*)");
        std::smatch match_count;
        if (std::regex_search(command, match_count, re_count)) {
            try {
                counter = boost::lexical_cast<decltype(counter)>(match_count[1].str());
                lights_->setCounter(counter);
            }
            catch(boost::bad_lexical_cast& ex) {
                std::cerr << "cannot read request <"<<command<<">\n";
            }
        }

        // need serialization to string here
        std::string msg("websocket echo: ");
        msg.append(command);

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write(
            boost::asio::buffer(std::string(msg)),
            boost::asio::bind_executor(
                strand_,
                std::bind(
                    &session::on_write,
                    shared_from_this(),
                    std::placeholders::_1,
                    std::placeholders::_2)));
    }

    void
    on_write(
        boost::system::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};



//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::shared_ptr<Lights> lights_;

public:
    listener(
        boost::asio::io_context& ioc,
        tcp::endpoint endpoint)
        : acceptor_(ioc)
        , socket_(ioc)
        , lights_(std::make_shared<Lights>())
    {
        boost::system::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            boost::asio::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        if(! acceptor_.is_open())
            return;
        do_accept();
    }

    void
    do_accept()
    {
        acceptor_.async_accept(
            socket_,
            std::bind(
                &listener::on_accept,
                shared_from_this(),
                std::placeholders::_1));
    }

    void
    on_accept(boost::system::error_code ec)
    {
        if(ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(std::move(socket_), lights_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // Check command line arguments.
    if (argc != 4)
    {
        std::cerr <<
            "Usage: websocket-server-async <address> <port> <threads>\n" <<
            "Example:\n" <<
            "    websocket-server-async 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = boost::asio::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    boost::asio::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
        [&ioc]
        {
            ioc.run();
        });
    ioc.run();

    return EXIT_SUCCESS;
}
