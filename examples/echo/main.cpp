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
  Client()
      : try_(0)
  {
    // nil
  };
  ~Client()
  {
    //
  }
  bool Init(const std::string &jid,
            const std::string &password,
            const std::string &server = "")
  {
    buzz::Jid tmp_jid(jid);
    if(!tmp_jid.IsValid()) {
      return false;
    }
    
    jid_.CopyFrom(tmp_jid);
    insecure_.password() = password;
    
    xcs_.set_user(jid_.node());
    xcs_.set_pass(talk_base::CryptString(insecure_));    
    xcs_.set_host(jid_.domain());
    xcs_.set_resource("chat");
    //xcs.set_use_tls(buzz::TLS_DISABLED);
    if("" != server) {
      xcs_.set_server(talk_base::SocketAddress(server, kDefaultXmppPort));
    } else {
      xcs_.set_server(talk_base::SocketAddress(jid_.domain(), kDefaultXmppPort));
    }
    talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
    SetResponse(true);
    thread_.reset(new echo::EchoThread);
    thread_->RegisterXmppHandler(this);
    thread_->Start();
    return true;
  }
  void Login()
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    thread_->Login(xcs_);
  }

  void Logout()
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    thread_->Disconnect();
    thread_->Stop();
  }

  virtual void DoOnXmppMessage(const buzz::Jid& from,
                               const buzz::Jid& to,
                               const std::string& message,
                               std::string* response)
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    LOG(LS_SENSITIVE) << message << " From " << from.Str();
    *response = message;
  }

  virtual void DoOnXmppOpen()
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    if(++try_ < 2) {
      thread_->Disconnect();      
    }
  }
  virtual void DoOnXmppClosed(int error)
  {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    LOG(LS_SENSITIVE) << "Error " << error;
    if(++try_ < 3) {
      Login();
    }
  }
 private:
  int try_;
  buzz::Jid jid_;
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
  Client client;

  client.Init(argv[1], argv[2], "113.142.30.52");
  client.Login();
  
  // Use main thread for console input
  std::string line;
  while (std::getline(std::cin, line)) {
    if ("quit" == line || "q" == line) {
      break;
    }
  }
  client.Logout();
  return 0;
}
