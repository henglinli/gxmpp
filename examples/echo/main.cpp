#include <iostream>
#include "talk/base/physicalsocketserver.h"
#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "echothread.h"

const int kDefaultXmppPort = 5222;

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << argv[0] << " jid password" << std::endl;
    return -1;
  }

  buzz::Jid jid(argv[1]);
  if(!jid.IsValid()) {
    std::cout << argv[1] << " bad jid" << std::endl;
    return -1;
  }

  talk_base::InsecureCryptStringImpl password;
  password.password() = argv[2];

  talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);

  bool reconnect = true;
  while (reconnect) {
    // Start xmpp on a different thread
    talk_base::PhysicalSocketServer ss;
    echo::EchoThread thread;
    thread.Start();

    // Create client settings
    buzz::XmppClientSettings xcs;
    xcs.set_user(jid.node());
    xcs.set_pass(talk_base::CryptString(password));
    xcs.set_host(jid.domain());
    xcs.set_resource("chat");
    //xcs.set_use_tls(buzz::TLS_DISABLED);
    xcs.set_server(talk_base::SocketAddress(jid.domain(), kDefaultXmppPort));

    thread.Login(xcs);

    // Use main thread for console input
    std::string line;
    while (std::getline(std::cin, line)) {
      if ("quit" == line || "q" == line) {
        reconnect = false;
      }
      if ("continue" == line || "c" == line || "q" == line || "quit" == line) {
        break;
      }
    }

    thread.Disconnect();
    thread.Stop();
  }

  return 0;
}
