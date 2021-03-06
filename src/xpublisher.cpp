/***************************************************************************
* Copyright (c) 2018, Johan Mabille, Sylvain Corlay and Martin Renou       *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <string>

#include "xpublisher.hpp"
#include "zmq_addon.hpp"
#include "xmiddleware.hpp"

namespace xeus
{
    xpublisher::xpublisher(zmq::context_t& context,
                           const std::string& transport,
                           const std::string& ip,
                           const std::string& port)
        : m_publisher(context, zmq::socket_type::pub)
        , m_listener(context, zmq::socket_type::sub)
        , m_controller(context, zmq::socket_type::rep)
    {
        init_socket(m_publisher, transport, ip, port);
        m_listener.connect(get_publisher_end_point());
        m_listener.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        m_controller.setsockopt(ZMQ_LINGER, get_socket_linger());
        m_controller.bind(get_publisher_controller_end_point());
    }

    xpublisher::~xpublisher()
    {
    }

    std::string xpublisher::get_port() const
    {
        return get_socket_port(m_publisher);
    }

    void xpublisher::run()
    {
        zmq::pollitem_t items[] = {
            { m_listener, 0, ZMQ_POLLIN, 0 },
            { m_controller, 0, ZMQ_POLLIN, 0 }
        };

        while (true)
        {
            zmq::poll(&items[0], 2, -1);

            if (items[0].revents & ZMQ_POLLIN)
            {
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_listener);
                wire_msg.send(m_publisher);
            }

            if (items[1].revents & ZMQ_POLLIN)
            {
                // stop message
                zmq::multipart_t wire_msg;
                wire_msg.recv(m_controller);
                wire_msg.send(m_controller);
                break;
            }
        }
    }
}
