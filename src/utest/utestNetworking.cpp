//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------
//
//  TSUnit test suite for networking classes.
//
//----------------------------------------------------------------------------

#include "tsIPUtils.h"
#include "tsIPAddress.h"
#include "tsIPv6Address.h"
#include "tsMACAddress.h"
#include "tsSocketAddress.h"
#include "tsTCPConnection.h"
#include "tsTCPServer.h"
#include "tsUDPSocket.h"
#include "tsThread.h"
#include "tsSysUtils.h"
#include "tsIPUtils.h"
#include "tsCerrReport.h"
#include "utestTSUnitThread.h"
#include "tsunit.h"
TSDUCK_SOURCE;


//----------------------------------------------------------------------------
// The test fixture
//----------------------------------------------------------------------------

class NetworkingTest: public tsunit::Test
{
public:
    NetworkingTest();

    virtual void beforeTest() override;
    virtual void afterTest() override;

    void testIPAddressConstructors();
    void testIPAddress();
    void testIPv6Address();
    void testMACAddress();
    void testGetLocalIPAddresses();
    void testSocketAddressConstructors();
    void testSocketAddress();
    void testTCPSocket();
    void testUDPSocket();
    void testIPHeader();

    TSUNIT_TEST_BEGIN(NetworkingTest);
    TSUNIT_TEST(testIPAddressConstructors);
    TSUNIT_TEST(testIPAddress);
    TSUNIT_TEST(testIPv6Address);
    TSUNIT_TEST(testMACAddress);
    TSUNIT_TEST(testGetLocalIPAddresses);
    TSUNIT_TEST(testSocketAddressConstructors);
    TSUNIT_TEST(testSocketAddress);
    TSUNIT_TEST(testTCPSocket);
    TSUNIT_TEST(testUDPSocket);
    TSUNIT_TEST(testIPHeader);
    TSUNIT_TEST_END();

private:
    int _previousSeverity;
};

TSUNIT_REGISTER(NetworkingTest);


//----------------------------------------------------------------------------
// Initialization.
//----------------------------------------------------------------------------

// Constructor.
NetworkingTest::NetworkingTest() :
    _previousSeverity(0)
{
}

// Test suite initialization method.
void NetworkingTest::beforeTest()
{
    _previousSeverity = CERR.maxSeverity();
    if (tsunit::Test::debugMode()) {
        CERR.setMaxSeverity(ts::Severity::Debug);
    }
}

// Test suite cleanup method.
void NetworkingTest::afterTest()
{
    CERR.setMaxSeverity(_previousSeverity);
}


//----------------------------------------------------------------------------
// Unitary tests.
//----------------------------------------------------------------------------

void NetworkingTest::testIPAddressConstructors()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    TSUNIT_ASSERT(ts::IPAddress::AnyAddress == 0);
    TSUNIT_ASSERT(ts::IPAddress::LocalHost.address() == 0x7F000001); // 127.0.0.1

    ts::IPAddress a1;
    TSUNIT_ASSERT(a1.address() == ts::IPAddress::AnyAddress);

    ts::IPAddress a2(0x01020304);
    TSUNIT_ASSERT(a2.address() == 0x01020304);

    ts::IPAddress a3(1, 2, 3, 4);
    TSUNIT_ASSERT(a3.address() == 0x01020304);

    ::in_addr ia4;
    ia4.s_addr = htonl(0x01020304);
    ts::IPAddress a4(ia4);
    TSUNIT_ASSERT(a4.address() == 0x01020304);

    ::sockaddr sa5;
    TSUNIT_ASSERT(sizeof(::sockaddr) >= sizeof(::sockaddr_in));
    ::sockaddr_in* sai5 = reinterpret_cast< ::sockaddr_in*> (&sa5);
    sai5->sin_family = AF_INET;
    sai5->sin_addr.s_addr = htonl (0x01020304);
    sai5->sin_port = 0;
    ts::IPAddress a5 (sa5);
    TSUNIT_ASSERT(a5.address() == 0x01020304);

    ::sockaddr_in sa6;
    sa6.sin_family = AF_INET;
    sa6.sin_addr.s_addr = htonl (0x01020304);
    sa6.sin_port = 0;
    ts::IPAddress a6 (sa6);
    TSUNIT_ASSERT(a6.address() == 0x01020304);

    ts::IPAddress a7(u"2.3.4.5");
    TSUNIT_ASSERT(a7.address() == 0x02030405);

    ts::IPAddress a8(u"localhost");
    TSUNIT_ASSERT(a8.address() == 0x7F000001); // 127.0.0.1
    TSUNIT_ASSERT(a8 == ts::IPAddress::LocalHost);
}

void NetworkingTest::testIPAddress()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    ts::IPAddress a1 (1, 2, 3, 4);
    ts::IPAddress a2 (1, 2, 3, 4);
    ts::IPAddress a3 (2, 3, 4, 5);

    TSUNIT_ASSERT(a1 == a2);
    TSUNIT_ASSERT(a1 != a3);

    a1.setAddress (0x02030405);
    TSUNIT_ASSERT(a1 == a3);

    a1.setAddress (1, 2, 3, 4);
    TSUNIT_ASSERT(a1 == a2);

    a2.setAddress (224, 1, 2, 3);
    TSUNIT_ASSERT(!a1.isMulticast());
    TSUNIT_ASSERT(a2.isMulticast());

    TSUNIT_ASSERT(a1.hasAddress());
    a1.clear();
    TSUNIT_ASSERT(!a1.hasAddress());
    TSUNIT_ASSERT(a1.address() == ts::IPAddress::AnyAddress);

    a1.setAddress(1, 2, 3, 4);
    ::in_addr ia;
    a1.copy(ia);
    TSUNIT_ASSERT(ia.s_addr == htonl(0x01020304));

    ::sockaddr sa;
    a1.copy (sa, 80);
    const ::sockaddr_in* saip = reinterpret_cast<const ::sockaddr_in*> (&sa);
    TSUNIT_ASSERT(saip->sin_family == AF_INET);
    TSUNIT_ASSERT(saip->sin_addr.s_addr == htonl (0x01020304));
    TSUNIT_ASSERT(saip->sin_port == htons (80));

    ::sockaddr_in sai;
    a1.copy (sai, 80);
    TSUNIT_ASSERT(sai.sin_family == AF_INET);
    TSUNIT_ASSERT(sai.sin_addr.s_addr == htonl (0x01020304));
    TSUNIT_ASSERT(sai.sin_port == htons (80));

    TSUNIT_ASSERT(a1.resolve(u"2.3.4.5"));
    TSUNIT_ASSERT(a1.address() == 0x02030405);

    TSUNIT_ASSERT(a1.resolve(u"localhost"));
    TSUNIT_ASSERT(a1.address() == 0x7F000001); // 127.0.0.1
    TSUNIT_ASSERT(a1 == ts::IPAddress::LocalHost);

    a1.setAddress(2, 3, 4, 5);
    const ts::UString s1(a1.toString());
    TSUNIT_ASSERT(s1 == u"2.3.4.5");

    debug() << "NetworkingTest: localhost = " << ts::IPAddress(u"localhost") << std::endl;

    // Note: fail if not connected to a network.
    debug() << "NetworkingTest: www.google.com = " << ts::IPAddress(u"www.google.com") << std::endl;
}

void NetworkingTest::testIPv6Address()
{
    ts::IPv6Address a1;
    TSUNIT_ASSERT(!a1.hasAddress());
    TSUNIT_ASSERT(!a1.isMulticast());

    TSUNIT_ASSERT(!ts::IPv6Address::AnyAddress.hasAddress());
    TSUNIT_ASSERT(ts::IPv6Address::LocalHost.hasAddress());
    TSUNIT_EQUAL(0, ts::IPv6Address::LocalHost.networkPrefix());
    TSUNIT_EQUAL(1, ts::IPv6Address::LocalHost.interfaceIdentifier());

    TSUNIT_ASSERT(!a1.resolve(u":", NULLREP));
    TSUNIT_ASSERT(!a1.hasAddress());

    TSUNIT_ASSERT(a1.resolve(u"::"));
    TSUNIT_ASSERT(!a1.hasAddress());
    TSUNIT_ASSERT(a1 == ts::IPv6Address::AnyAddress);

    TSUNIT_ASSERT(a1.resolve(u"::1"));
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_ASSERT(a1 == ts::IPv6Address::LocalHost);

    TSUNIT_ASSERT(!a1.resolve(u"", NULLREP));
    TSUNIT_ASSERT(!a1.hasAddress());

    a1.setAddress(0, 1, 2, 3, 4, 5, 6, 7);
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_EQUAL(TS_UCONST64(0x0000000100020003), a1.networkPrefix());
    TSUNIT_EQUAL(TS_UCONST64(0x0004000500060007), a1.interfaceIdentifier());
    TSUNIT_EQUAL(u"0:1:2:3:4:5:6:7", a1.toString());
    TSUNIT_EQUAL(u"0000:0001:0002:0003:0004:0005:0006:0007", a1.toFullString());

    a1.setAddress(0x12, 0x345, 0x6789, 0xFFFF, 0, 0, 0, 0xBEEF);
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_EQUAL(TS_UCONST64(0x001203456789FFFF), a1.networkPrefix());
    TSUNIT_EQUAL(TS_UCONST64(0x000000000000BEEF), a1.interfaceIdentifier());
    TSUNIT_EQUAL(u"12:345:6789:ffff::beef", a1.toString());
    TSUNIT_EQUAL(u"0012:0345:6789:ffff:0000:0000:0000:beef", a1.toFullString());

    TSUNIT_ASSERT(a1.resolve(u"fe80::93a3:dea0:2108:b81e"));
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_EQUAL(TS_UCONST64(0xFE80000000000000), a1.networkPrefix());
    TSUNIT_EQUAL(TS_UCONST64(0x93A3DEA02108B81E), a1.interfaceIdentifier());
    TSUNIT_EQUAL(u"fe80::93a3:dea0:2108:b81e", a1.toString());
    TSUNIT_EQUAL(u"fe80:0000:0000:0000:93a3:dea0:2108:b81e", a1.toFullString());
}

void NetworkingTest::testMACAddress()
{
    ts::MACAddress a1;
    TSUNIT_ASSERT(!a1.hasAddress());
    TSUNIT_ASSERT(!a1.isMulticast());

    TSUNIT_ASSERT(a1.resolve(u"52:54:00:26:92:b4"));
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_ASSERT(!a1.isMulticast());
    TSUNIT_EQUAL(TS_UCONST64(0x5254002692B4), a1.address());
    TSUNIT_EQUAL(u"52:54:00:26:92:B4", a1.toString());

    TSUNIT_ASSERT(a1.resolve(u" 23:b3-A6 . bE : 56-4D"));
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_ASSERT(!a1.isMulticast());
    TSUNIT_EQUAL(TS_UCONST64(0x23B3A6BE564D), a1.address());
    TSUNIT_EQUAL(u"23:B3:A6:BE:56:4D", a1.toString());

    TSUNIT_ASSERT(a1.toMulticast(ts::IPAddress(225, 1, 2, 3)));
    TSUNIT_ASSERT(a1.hasAddress());
    TSUNIT_ASSERT(a1.isMulticast());
    TSUNIT_EQUAL(TS_UCONST64(0x01005E010203), a1.address());
    TSUNIT_EQUAL(u"01:00:5E:01:02:03", a1.toString());

    TSUNIT_ASSERT(!a1.toMulticast(ts::IPAddress(192, 168, 2, 3)));
    TSUNIT_ASSERT(!a1.hasAddress());
    TSUNIT_ASSERT(!a1.isMulticast());
}

void NetworkingTest::testGetLocalIPAddresses()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    // We cannot assume that the local system has any local address.
    // We only requires that the call does not fail.
    ts::IPAddressVector addr;
    TSUNIT_ASSERT(ts::GetLocalIPAddresses(addr));

    ts::IPAddressMaskVector addrMask;
    TSUNIT_ASSERT(ts::GetLocalIPAddresses(addrMask));

    // The two calls must return the same number of addresses.
    TSUNIT_ASSERT(addr.size() == addrMask.size());

    debug() << "NetworkingTest: GetLocalIPAddresses: " << addrMask.size() << " local addresses" << std::endl;
    for (size_t i = 0; i < addrMask.size(); ++i) {
        debug() << "NetworkingTest: local address " << i
                << ": " << addrMask[i].address
                << ", mask: " << addrMask[i].mask
                << ", broadcast: " << addrMask[i].broadcastAddress()
                << " (" << addrMask[i] << ")" << std::endl;
    }

    for (size_t i = 0; i < addr.size(); ++i) {
        TSUNIT_ASSERT(ts::IsLocalIPAddress(addr[i]));
    }

    for (size_t i = 0; i < addrMask.size(); ++i) {
        TSUNIT_ASSERT(ts::IsLocalIPAddress(addrMask[i].address));
    }
}

void NetworkingTest::testSocketAddressConstructors()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    TSUNIT_ASSERT(ts::SocketAddress::AnyAddress == 0);
    TSUNIT_ASSERT(ts::SocketAddress::LocalHost.address() == 0x7F000001); // 127.0.0.1

    ts::SocketAddress a1;
    TSUNIT_ASSERT(a1.address() == ts::SocketAddress::AnyAddress);
    TSUNIT_ASSERT(a1.port() == ts::SocketAddress::AnyPort);

    ts::SocketAddress a2a (ts::IPAddress(0x01020304), 80);
    TSUNIT_ASSERT(a2a.address() == 0x01020304);
    TSUNIT_ASSERT(a2a.port() == 80);

    ts::SocketAddress a2b (0x01020304, 80);
    TSUNIT_ASSERT(a2b.address() == 0x01020304);
    TSUNIT_ASSERT(a2b.port() == 80);

    ts::SocketAddress a3 (1, 2, 3, 4, 80);
    TSUNIT_ASSERT(a3.address() == 0x01020304);
    TSUNIT_ASSERT(a3.port() == 80);

    ::in_addr ia4;
    ia4.s_addr = htonl (0x01020304);
    ts::SocketAddress a4 (ia4, 80);
    TSUNIT_ASSERT(a4.address() == 0x01020304);
    TSUNIT_ASSERT(a4.port() == 80);

    ::sockaddr sa5;
    TSUNIT_ASSERT(sizeof(::sockaddr) >= sizeof(::sockaddr_in));
    ::sockaddr_in* sai5 = reinterpret_cast< ::sockaddr_in*> (&sa5);
    sai5->sin_family = AF_INET;
    sai5->sin_addr.s_addr = htonl (0x01020304);
    sai5->sin_port = htons (80);
    ts::SocketAddress a5 (sa5);
    TSUNIT_ASSERT(a5.address() == 0x01020304);
    TSUNIT_ASSERT(a5.port() == 80);

    ::sockaddr_in sa6;
    sa6.sin_family = AF_INET;
    sa6.sin_addr.s_addr = htonl(0x01020304);
    sa6.sin_port = htons(80);
    ts::SocketAddress a6(sa6);
    TSUNIT_ASSERT(a6.address() == 0x01020304);
    TSUNIT_ASSERT(a6.port() == 80);

    ts::SocketAddress a7(u"2.3.4.5");
    TSUNIT_ASSERT(a7.address() == 0x02030405);
    TSUNIT_ASSERT(a7.port() == ts::SocketAddress::AnyPort);

    ts::SocketAddress a8(u"localhost");
    TSUNIT_ASSERT(a8.address() == 0x7F000001); // 127.0.0.1
    TSUNIT_ASSERT(a8 == ts::IPAddress::LocalHost);
    TSUNIT_ASSERT(a8.port() == ts::SocketAddress::AnyPort);

    ts::SocketAddress a9(u"2.3.4.5:80");
    TSUNIT_ASSERT(a9.address() == 0x02030405);
    TSUNIT_ASSERT(a9.port() == 80);

    ts::SocketAddress a10(u":80");
    TSUNIT_ASSERT(a10.address() == ts::IPAddress::AnyAddress);
    TSUNIT_ASSERT(a10.port() == 80);
}

void NetworkingTest::testSocketAddress()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    ts::SocketAddress a1(1, 2, 3, 4, 80);
    ts::SocketAddress a2(1, 2, 3, 4, 80);
    ts::SocketAddress a3(1, 3, 4, 5, 81);

    TSUNIT_ASSERT(a1 == a2);
    TSUNIT_ASSERT(a1 != a3);

    a1.setAddress(1, 3, 4, 5);
    a1.setPort(81);
    TSUNIT_ASSERT(a1 == a3);

    a1.setPort(80);
    a1.setAddress(1, 2, 3, 4);
    TSUNIT_ASSERT(a1 == a2);

    a2.set(5, 1, 2, 3, 8080);
    TSUNIT_ASSERT(a2.address() == 0x05010203);
    TSUNIT_ASSERT(a2.port() == 8080);

    TSUNIT_ASSERT(a2.hasAddress());
    TSUNIT_ASSERT(a2.hasPort());
    a2.clear();
    TSUNIT_ASSERT(!a2.hasAddress());
    TSUNIT_ASSERT(!a2.hasPort());
    TSUNIT_ASSERT(a2.address() == ts::IPAddress::AnyAddress);
    TSUNIT_ASSERT(a2.port() == ts::SocketAddress::AnyPort);

    a1.set(1, 2, 3, 4, 80);
    ::in_addr ia;
    a1.copy(ia);
    TSUNIT_ASSERT(ia.s_addr == htonl(0x01020304));

    ::sockaddr sa;
    a1.copy(sa);
    const ::sockaddr_in* saip = reinterpret_cast<const ::sockaddr_in*> (&sa);
    TSUNIT_ASSERT(saip->sin_family == AF_INET);
    TSUNIT_ASSERT(saip->sin_addr.s_addr == htonl(0x01020304));
    TSUNIT_ASSERT(saip->sin_port == htons(80));

    ::sockaddr_in sai;
    a1.copy(sai);
    TSUNIT_ASSERT(sai.sin_family == AF_INET);
    TSUNIT_ASSERT(sai.sin_addr.s_addr == htonl(0x01020304));
    TSUNIT_ASSERT(sai.sin_port == htons(80));

    a1.set(2, 3, 4, 5, 80);
    const ts::UString s1(a1.toString());
    TSUNIT_ASSERT(s1 == u"2.3.4.5:80");

    a1.clearPort();
    const ts::UString s2(a1.toString());
    TSUNIT_ASSERT(s2 == u"2.3.4.5");
}

// A thread class which implements a TCP/IP client.
// It sends one message and wait from the same message to be replied.
namespace {
    class TCPClient: public utest::TSUnitThread
    {
        TS_NOBUILD_NOCOPY(TCPClient);
    private:
        uint16_t _portNumber;
    public:
        // Constructor
        explicit TCPClient(uint16_t portNumber) :
            utest::TSUnitThread(),
            _portNumber(portNumber)
        {
        }

        // Destructor
        ~TCPClient()
        {
            waitForTermination();
            CERR.debug(u"TCPSocketTest: client thread: destroyed");
        }

        // Thread execution
        virtual void test() override
        {
            CERR.debug(u"TCPSocketTest: client thread: started");

            // Connect to the server.
            const ts::SocketAddress serverAddress(ts::IPAddress::LocalHost, _portNumber);
            const ts::SocketAddress clientAddress(ts::IPAddress::LocalHost, ts::SocketAddress::AnyPort);
            ts::TCPConnection session;
            TSUNIT_ASSERT(!session.isOpen());
            TSUNIT_ASSERT(!session.isConnected());
            TSUNIT_ASSERT(session.open(CERR));
            TSUNIT_ASSERT(session.setSendBufferSize(1024, CERR));
            TSUNIT_ASSERT(session.setReceiveBufferSize(1024, CERR));
            TSUNIT_ASSERT(session.bind(clientAddress, CERR));
            TSUNIT_ASSERT(session.connect(serverAddress, CERR));
            TSUNIT_ASSERT(session.isOpen());
            TSUNIT_ASSERT(session.isConnected());

            ts::SocketAddress peer;
            TSUNIT_ASSERT(session.getPeer(peer, CERR));
            TSUNIT_ASSERT(peer == serverAddress);
            TSUNIT_ASSERT(ts::IPAddress(peer) == ts::IPAddress::LocalHost);
            TSUNIT_ASSERT(peer.port() == _portNumber);

            // Send a message
            const char message[] = "Hello";
            CERR.debug(u"TCPSocketTest: client thread: sending \"%s\", %d bytes", {message, sizeof(message)});
            TSUNIT_ASSERT(session.send(message, sizeof(message), CERR));
            CERR.debug(u"TCPSocketTest: client thread: data sent");

            // Say we won't send no more
            TSUNIT_ASSERT(session.closeWriter(CERR));

            // Loop until on server response.
            size_t totalSize = 0;
            char buffer [1024];
            size_t size = 0;
            while (totalSize < sizeof(buffer) && session.receive(buffer + totalSize, sizeof(buffer) - totalSize, size, nullptr, CERR)) {
                CERR.debug(u"TCPSocketTest: client thread: data received, %d bytes", {size});
                totalSize += size;
            }
            CERR.debug(u"TCPSocketTest: client thread: end of data stream");
            TSUNIT_ASSERT(totalSize == sizeof(message));
            TSUNIT_ASSERT(::memcmp(message, buffer, totalSize) == 0);

            // Fully disconnect the session
            session.disconnect(CERR);
            session.close(CERR);
            CERR.debug(u"TCPSocketTest: client thread: terminated");
        }
    };
}

// Test cases
void NetworkingTest::testTCPSocket()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    const uint16_t portNumber = 12345;

    // Create server socket
    CERR.debug(u"TCPSocketTest: main thread: create server");
    const ts::SocketAddress serverAddress(ts::IPAddress::LocalHost, portNumber);
    ts::TCPServer server;
    TSUNIT_ASSERT(!server.isOpen());
    TSUNIT_ASSERT(server.open(CERR));
    TSUNIT_ASSERT(server.isOpen());
    TSUNIT_ASSERT(server.reusePort(true, CERR));
    TSUNIT_ASSERT(server.setSendBufferSize(1024, CERR));
    TSUNIT_ASSERT(server.setReceiveBufferSize(1024, CERR));
    TSUNIT_ASSERT(server.setTTL(1, CERR));
    TSUNIT_ASSERT(server.bind (serverAddress, CERR));
    TSUNIT_ASSERT(server.listen(5, CERR));

    CERR.debug(u"TCPSocketTest: main thread: starting client thread");
    TCPClient client(portNumber);
    client.start();

    CERR.debug(u"TCPSocketTest: main thread: waiting for a client");
    ts::TCPConnection session;
    ts::SocketAddress clientAddress;
    TSUNIT_ASSERT(server.accept(session, clientAddress, CERR));
    CERR.debug(u"TCPSocketTest: main thread: got a client");
    TSUNIT_ASSERT(ts::IPAddress(clientAddress) == ts::IPAddress::LocalHost);

    CERR.debug(u"TCPSocketTest: main thread: waiting for data");
    ts::SocketAddress sender;
    char buffer [1024];
    size_t size = 0;
    while (session.receive(buffer, sizeof(buffer), size, nullptr, CERR)) {
        CERR.debug(u"TCPSocketTest: main thread: data received, %d bytes", {size});
        TSUNIT_ASSERT(session.send(buffer, size, CERR));
        CERR.debug(u"TCPSocketTest: main thread: data sent back");
    }

    CERR.debug(u"TCPSocketTest: main thread: end of client session");
    session.disconnect(CERR);
    session.close(CERR);
    TSUNIT_ASSERT(server.close(CERR));

    CERR.debug(u"TCPSocketTest: main thread: terminated");
}

// A thread class which sends one UDP message and wait from the same message to be replied.
namespace {
    class UDPClient: public utest::TSUnitThread
    {
        TS_NOBUILD_NOCOPY(UDPClient);
    private:
        uint16_t _portNumber;
    public:
        // Constructor
        explicit UDPClient(uint16_t portNumber) :
            utest::TSUnitThread(),
            _portNumber(portNumber)
        {
        }

        // Destructor
        ~UDPClient()
        {
            waitForTermination();
            CERR.debug(u"UDPSocketTest: client thread destroyed");
        }

        // Thread execution
        virtual void test() override
        {
            CERR.debug(u"UDPSocketTest: client thread started");
            // Create the client socket
            ts::UDPSocket sock(true);
            TSUNIT_ASSERT(sock.isOpen());
            TSUNIT_ASSERT(sock.setSendBufferSize(1024, CERR));
            TSUNIT_ASSERT(sock.setReceiveBufferSize(1024, CERR));
            TSUNIT_ASSERT(sock.bind(ts::SocketAddress(ts::IPAddress::LocalHost, ts::SocketAddress::AnyPort), CERR));
            TSUNIT_ASSERT(sock.setDefaultDestination(ts::SocketAddress(ts::IPAddress::LocalHost, _portNumber), CERR));
            TSUNIT_ASSERT(ts::IPAddress(sock.getDefaultDestination()) == ts::IPAddress::LocalHost);
            TSUNIT_ASSERT(sock.getDefaultDestination().port() == _portNumber);

            // Send a message
            const char message[] = "Hello";
            CERR.debug(u"UDPSocketTest: client thread: sending \"%s\", %d bytes", {message, sizeof(message)});
            TSUNIT_ASSERT(sock.send(message, sizeof(message), CERR));
            CERR.debug(u"UDPSocketTest: client thread: request sent");

            // Wait for a reply
            ts::SocketAddress sender;
            ts::SocketAddress destination;
            char buffer [1024];
            size_t size;
            TSUNIT_ASSERT(sock.receive(buffer, sizeof(buffer), size, sender, destination, nullptr, CERR));
            CERR.debug(u"UDPSocketTest: client thread: reply received, %d bytes, sender: %s, destination: %s", {size, sender, destination});
            TSUNIT_ASSERT(size == sizeof(message));
            TSUNIT_ASSERT(::memcmp(message, buffer, size) == 0);
            TSUNIT_ASSERT(ts::IPAddress(sender) == ts::IPAddress::LocalHost);
            TSUNIT_ASSERT(sender.port() == _portNumber);

            CERR.debug(u"UDPSocketTest: client thread terminated");
        }
    };
}

// Test cases
void NetworkingTest::testUDPSocket()
{
    TSUNIT_ASSERT(ts::IPInitialize());

    const uint16_t portNumber = 12345;

    // Create server socket
    ts::UDPSocket sock;
    TSUNIT_ASSERT(!sock.isOpen());
    TSUNIT_ASSERT(sock.open(CERR));
    TSUNIT_ASSERT(sock.isOpen());
    TSUNIT_ASSERT(sock.setSendBufferSize(1024, CERR));
    TSUNIT_ASSERT(sock.setReceiveBufferSize(1024, CERR));
    TSUNIT_ASSERT(sock.reusePort(true, CERR));
    TSUNIT_ASSERT(sock.setTTL(1, false, CERR));
    TSUNIT_ASSERT(sock.bind(ts::SocketAddress(ts::IPAddress::LocalHost, portNumber), CERR));

    CERR.debug(u"UDPSocketTest: main thread: starting client thread");
    UDPClient client(portNumber);
    client.start();

    CERR.debug(u"UDPSocketTest: main thread: waiting for message");
    ts::SocketAddress sender;
    ts::SocketAddress destination;
    char buffer [1024];
    size_t size;
    TSUNIT_ASSERT(sock.receive(buffer, sizeof(buffer), size, sender, destination, nullptr, CERR));
    CERR.debug(u"UDPSocketTest: main thread: request received, %d bytes, sender: %s, destination: %s", {size, sender, destination});
    TSUNIT_ASSERT(ts::IPAddress(sender) == ts::IPAddress::LocalHost);

    TSUNIT_ASSERT(sock.send(buffer, size, sender, CERR));
    CERR.debug(u"UDPSocketTest: main thread: reply sent");
}

// Test IP header
void NetworkingTest::testIPHeader()
{
    static const uint8_t reference_header[] = {
        0x45, 0x00, 0x05, 0xBE, 0xFB, 0x6E, 0x00, 0x00, 0x32, 0x06,
        0x32, 0x8B, 0xD8, 0x3A, 0xCC, 0x8E, 0xAC, 0x14, 0x04, 0x63,
    };

    TSUNIT_EQUAL(sizeof(reference_header), ts::IPHeaderSize(reference_header, sizeof(reference_header)));
    TSUNIT_EQUAL(0x328B, ts::IPHeaderChecksum(reference_header, sizeof(reference_header)));
    TSUNIT_ASSERT(ts::VerifyIPHeaderChecksum(reference_header, sizeof(reference_header)));

    uint8_t header[sizeof(reference_header)];
    ::memcpy(header, reference_header, sizeof(header));

    TSUNIT_ASSERT(ts::VerifyIPHeaderChecksum(header, sizeof(header)));
    header[ts::IPv4_CHECKSUM_OFFSET] = 0x00;
    header[ts::IPv4_CHECKSUM_OFFSET + 1] = 0x00;
    TSUNIT_ASSERT(!ts::VerifyIPHeaderChecksum(header, sizeof(header)));
    TSUNIT_EQUAL(0x328B, ts::IPHeaderChecksum(header, sizeof(header)));

    TSUNIT_ASSERT(ts::UpdateIPHeaderChecksum(header, sizeof(header)));
    TSUNIT_ASSERT(ts::VerifyIPHeaderChecksum(header, sizeof(header)));
    TSUNIT_EQUAL(0x328B, ts::IPHeaderChecksum(header, sizeof(header)));
}
