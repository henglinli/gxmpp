#include <iostream>
#include "talk/base/physicalsocketserver.h"
#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "echothread.h"

const int kDefaultXmppPort = 5222;

class Client : public echo::XmppHandler
{
 public:
  Client(const std::string & jid, const std::string & password)
      : try_(0)
      , jid_(jid)
      , password_(password)
  {
    // nil
  };
  ~Client()
  {
    //
  }
  void Init()
  {
    insecure_.password() = password_;
    
    xcs_.set_user(jid_.node());
    xcs_.set_pass(talk_base::CryptString(insecure_));
    xcs_.set_host(jid_.domain());
    xcs_.set_resource("chat");
    //xcs.set_use_tls(buzz::TLS_DISABLED);
    xcs_.set_server(talk_base::SocketAddress(jid_.domain(), kDefaultXmppPort));

    talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
   
  }
  void Login()
  {
    thread_.reset(new echo::EchoThread);
    thread_->RegisterXmppHandler(this);    
    thread_->Start();
    thread_->Login(xcs_);
  }
  void Logout()
  {
    thread_->Disconnect();
    thread_->Stop();
  }
  virtual void OnXmppMessage(const buzz::Jid& from,
                             const buzz::Jid& to,
                             const std::string& message,
                             std::string* response)
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    LOG(LS_SENSITIVE) << message << " From " << from.Str() << " To " << to.Str();
    *response = message;
  }
  virtual void OnXmppOpen()
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    if(++try_ < 2) {
      Logout();
    }
  }
  virtual void OnXmppClosed(int error)
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    LOG(LS_SENSITIVE) << "Error " << error;
    Login();
  }
 private:
  int try_;
  buzz::Jid jid_;
  std::string password_;
  talk_base::InsecureCryptStringImpl insecure_;
  talk_base::scoped_ptr<echo::EchoThread> thread_;
  buzz::XmppClientSettings xcs_;
};

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
  Client client(argv[1], argv[2]);

  client.Init();
  client.Login();
  
  // Use main thread for console input
  std::string line;
  while (std::getline(std::cin, line)) {
    if ("quit" == line || "q" == line) {
      break;
    }
  }
#if 0
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

    thread.RegisterXmppHandler(&handler);
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
#endif
  return 0;
}
