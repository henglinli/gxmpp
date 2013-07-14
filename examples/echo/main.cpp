/*
 * gxmpp
 * Copyright 2013, Henry Lee <henglinli@gmail.com>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "talk/base/physicalsocketserver.h"
#include "talk/base/cryptstring.h"
#include "talk/base/logging.h"
#include "talk/xmpp/xmppclientsettings.h"

#include "gxmpp/pumpthread.h"
#include "echothread.h"

//#define HEAPCHECK
#ifdef HEAPCHECK
#include "gperftools/heap-checker.h"
#endif

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
  void Disconnect() {
    LOG(LS_SENSITIVE) << __PRETTY_FUNCTION__;
    thread_->Disconnect();
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
  virtual void DoOnXmppClosed(buzz::XmppEngine::Error error)
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
#ifdef HEAPCHECK
  HeapLeakChecker heap_checker("check_gxmpp");
#endif
  {
    if (argc != 3) {
      std::cout << argv[0] << " jid password" << std::endl;
      return -1;
    }

    buzz::Jid jid(argv[1]);
    if(!jid.IsValid()) {
      std::cout << argv[1] << " bad jid" << std::endl;
      return -1;
    }
    talk_base::LogMessage::LogToDebug(talk_base::LS_SENSITIVE);
    //#define XMPPTHREAD
#ifndef XMPPTHREAD
    Client client;
    
    client.Init(argv[1], argv[2], "113.142.30.52");
    client.Login();
    // Use main thread for console input
    std::string line;
    while (std::getline(std::cin, line)) {
      if ("quit" == line || "q" == line) {
        break;
      }
      if("continue" == line || "c" == line) {
        client.Disconnect();
        client.Login();
      }
    }
    client.Logout();
#else
    gxmpp::PumpThread thread;
    thread.Start();
    thread.Init(argv[1], argv[2], "113.142.30.52");
    thread.Login();
    // Use main thread for console input
    std::string line;
    while (std::getline(std::cin, line)) {
      if ("quit" == line || "q" == line) {
        break;
      }
      if("continue" == line || "c" == line) {
        thread.Disconnect();
        thread.Login();
      }
    }
    thread.Disconnect();
#endif
  }
#ifdef HEAPCHECK
  heap_checker.NoLeaks();
#endif
  return 0;
}
